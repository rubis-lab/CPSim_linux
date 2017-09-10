#include "stdafx.h"
#include "task_created.hh"
#include "can_api.h"
#include "data_list.hh"
#include <sys/time.h>
#include <unistd.h>

#define TIME_WINDOW	1000000		// us
#define OVER_TIME 1000000		// us

#define LOCATION "~/"	// path for log

int nType = HW_PCI;
__u32 dwPort = 0;
__u16 wIrq = 0;
__u16 wBTR0BTR1 = CAN_BAUD_1M;
int nExtended = CAN_INIT_TYPE_ST;

int num_resources = 0;
int num_tasks = 0;
vector<Resource*> resources;
vector<Task_info*> whole_tasks;
Plan *plan;
Execution *execution;

int current_time = 0;		// simulation time (us)
timeval reference_time;
timeval now;				// real time

// for data from CAR
float car_output[10];
float user_input[10];

// for internal data
float memory_buffer[10];

// for sending data
pthread_mutex_t section_for_sending = PTHREAD_MUTEX_INITIALIZER;
list<CAN_Msg *> waiting_data;

// data structure for schedule plotting
pthread_mutex_t section_for_waiting = PTHREAD_MUTEX_INITIALIZER;
list<Time_plot *> waiting_plot;
int recent_time = 0;	// time when can packet was sent recently

// data structure for response time plotting
pthread_mutex_t section_for_response = PTHREAD_MUTEX_INITIALIZER;
list<Time_plot *> response_plot;

// for real-time plotting
unsigned long long passed_time = 0;
FILE *acc, *str, *spd, *rpm, *yaw;
FILE *fp_chart, *fp_serial;
FILE *response_chart[MAX_TASKS];

// for infinite looping
int loop_condition = 1;

// for CAN
HANDLE hCAN1;
HANDLE hCAN2;

void initialize();
void init_CAN(int num_channel);
void initialize_for_plotting();
unsigned long long getcurrenttime();
void wait(unsigned long long to);
void* receive_CAN_thread(void *arg);
void try_send_data_via_can(FILE *fp);
void try_write_for_schedule_plot(FILE *fp);
void try_write_for_line_plot();
void try_write_for_response_plot();
void program_complete();
void add_time_plot(Task *task, list<Time_plot *> *plot);

// main function
int main(int argc, char* argv[])
{
	// create resources, schedulers, tasks and set parameters
	initialize();

	// set CAN connection
	init_CAN(1);

	// initialize for plotting
	initialize_for_plotting();

	// create recv thread
	pthread_t hThread;
	int thread_id;
	thread_id = pthread_create(&hThread, NULL, receive_CAN_thread, NULL);	
	if(thread_id < 0) exit(3);  

	char *task_name;
	char *resource_name;
	int time_print;
	int is_start;

	Task *next = NULL;

	// check current time
	gettimeofday(&reference_time, NULL);
	unsigned long long expected_end_time = 0;
	unsigned long long actual_start_time = 0;

	// main loop
	while(loop_condition == 1)
	{
		// try sending data
		try_send_data_via_can(fp_chart);

		// try writing for schedule plot
		try_write_for_schedule_plot(fp_chart);

		// try writing for line plot
		try_write_for_line_plot();

		// try writing for response plot
		try_write_for_response_plot();

		int current_time_temp = current_time;

		// increase simulation time
		current_time += TIME_WINDOW;

		// create expected schedule for each resource
		for(unsigned int i = 0; i < resources.size(); i++)
		{
			resources[i]->scheduler_link->extract_schedule(current_time, OVER_TIME, &waiting_plot);
		}

		// optimize waiting jobs with considering effective release time and deadline
		plan->generate_plan(&resources, &whole_tasks);
		while(loop_condition == 1)
		{
			// try sending data
			try_send_data_via_can(fp_chart);

			// try writing for schedule plot
			try_write_for_schedule_plot(fp_chart);

			// try writing for line plot
			try_write_for_line_plot();

			// try writing for response plot
			try_write_for_response_plot();

			// get next executable job
			next = execution->get_next_job(current_time_temp, &whole_tasks);
			if(next == NULL)
				break;

			if(next->task_link->get_is_read() == 1)		// wait for time synchronization
			{
				do
				{
					try_send_data_via_can(fp_chart);
					try_write_for_schedule_plot(fp_chart);
					try_write_for_line_plot();
					try_write_for_response_plot();
				} while(next->get_effective_release_time() > getcurrenttime());
			}

			// calculate expected start and end time
			actual_start_time = current_time_temp;
			expected_end_time = actual_start_time + next->task_link->get_modified_wcet();

			// print seriaized schedule (start)
			task_name = next->task_link->get_task_name();
			resource_name = next->task_link->scheduler_link->resource_link->get_resource_name();
			is_start = 1;
			time_print = actual_start_time/1000;
			fprintf(fp_serial, "%d, %s: %s, %d\n", time_print, resource_name, task_name, is_start);
			fflush(fp_serial);

			// push response time for plotting
			pthread_mutex_lock(&section_for_response);
			add_time_plot(next, &response_plot);
			pthread_mutex_unlock(&section_for_response);

			// jump simulation time into this job's release time
			if(next->task_link->get_is_read() == 1 && current_time_temp < next->get_effective_release_time())
				current_time_temp = next->get_effective_release_time() + next->task_link->get_modified_wcet();
			else
				current_time_temp += next->task_link->get_modified_wcet();

			// execute a task (job)
			next->read();
			next->procedure();
			next->write();

			// print seriaized schedule
			task_name = next->task_link->get_task_name();
			resource_name = next->task_link->scheduler_link->resource_link->get_resource_name();
			is_start = 0;
			time_print = expected_end_time/1000;
			fprintf(fp_serial, "%d, %s: %s, %d\n", time_print, resource_name, task_name, is_start);
			fflush(fp_serial);

			delete next;

			// wait during modified wcet
			do
			{
				try_send_data_via_can(fp_chart);
				try_write_for_schedule_plot(fp_chart);
				try_write_for_line_plot();
				try_write_for_response_plot();
			} while(expected_end_time > getcurrenttime());
		}
	}

	// uninitialize phase
	fclose(fp_chart);
	fclose(fp_serial);
	CAN_Close(hCAN1);
	CAN_Close(hCAN2);
	getchar();

	return 0;
}

/* This function initializes the simulator.
 * In this function, required memory section are initialized.
 */
void initialize()
{
#include "initialize.hh"

	num_resources = resources.size();
	num_tasks = whole_tasks.size();

	// set priority for fixed priority task sets
	for(int i = 0; i < num_resources; i++)
		resources[i]->scheduler_link->set_priority();

	// ready for plan and execution
	plan = new Plan();
	execution  = new Execution();

	// for data from car
	for(int i = 0; i < 10; i++)
	{
		car_output[i] = 0.0;
		memory_buffer[i] = 0.0;
	}
}

/* This function initializes CAN connection.
 *
 * <argument>
 * num_channel: number of CAN channels
 */
void init_CAN(int num_channel)
{
	// can connect
	int i = 0;

	hCAN1 = LINUX_CAN_Open(PCANUSB1, O_RDWR);
	if(!hCAN1)
	{
		cerr << "No device, USBBUS1\n";
		exit(1);
	}
	if(wBTR0BTR1)
	{
		errno = CAN_Init(hCAN1, wBTR0BTR1, nExtended);
		if(errno)
		{
			CAN_Close(hCAN1);
			return;
		}	
	}
	CAN_Status(hCAN1);

	if(num_channel <= 1) return;

	hCAN2 = LINUX_CAN_Open(PCANUSB2, O_RDWR);
	if(!hCAN2)
	{
		cerr << "No device, USBBUS2\n";
		exit(1);
	}
	if(wBTR0BTR1)
	{
		errno = CAN_Init(hCAN2, wBTR0BTR1, nExtended);
		if(errno)
		{
			CAN_Close(hCAN2);
			return;
		}	
	}
}

/* This function initializes the files for plotting.
 */
void initialize_for_plotting()
{
	char file_name[100];

	// for data plotting
	sprintf(file_name, "%sacceleration.txt", LOCATION);
	acc = fopen(file_name, "w");
	sprintf(file_name, "%ssteering.txt", LOCATION);
	str = fopen(file_name, "w");
	sprintf(file_name, "%sspeed.txt", LOCATION);
	spd = fopen(file_name, "w");
	sprintf(file_name, "%srpm.txt", LOCATION);
	rpm = fopen(file_name, "w");
	sprintf(file_name, "%syaw.txt", LOCATION);
	yaw = fopen(file_name, "w");

	// for expected schedule plotting
	sprintf(file_name, "%sinternal.log", LOCATION);
	fp_chart = fopen(file_name, "w");
	fprintf(fp_chart, "%s: %s", whole_tasks[0]->scheduler_link->resource_link->get_resource_name(), whole_tasks[0]->get_task_name());
	if(whole_tasks[0]->get_is_write() == 1)
		fprintf(fp_chart, ", msg: %s", whole_tasks[0]->get_task_name());
	for(unsigned int i = 1; i < whole_tasks.size(); i++)
	{
		fprintf(fp_chart, ", %s: %s", whole_tasks[i]->scheduler_link->resource_link->get_resource_name(), whole_tasks[i]->get_task_name());
		if(whole_tasks[i]->get_is_write() == 1)
			fprintf(fp_chart, ", msg: %s", whole_tasks[i]->get_task_name());
	}
	fprintf(fp_chart, "\n");
	fflush(fp_chart);

	// for serialized schedule plotting
	sprintf(file_name, "%sserial.log", LOCATION);
	fp_serial = fopen(file_name, "w");
	fprintf(fp_serial, "%s: %s", whole_tasks[0]->scheduler_link->resource_link->get_resource_name(), whole_tasks[0]->get_task_name());
	for(unsigned int i = 1; i < whole_tasks.size(); i++)
		fprintf(fp_serial, ", %s: %s", whole_tasks[i]->scheduler_link->resource_link->get_resource_name(), whole_tasks[i]->get_task_name());
	fprintf(fp_serial, "\n");
	fflush(fp_serial);

	// for response time plotting
	for(int i = 0; i < num_tasks; i++)
	{
		sprintf(file_name, "%stask%d.txt", LOCATION, i);
		response_chart[i] = fopen(file_name, "w");
	}

	// for config.txt
	sprintf(file_name, "%sconfig.txt", LOCATION);
	FILE *fp_config = fopen(file_name, "w");
	FILE *fp_original = fopen("config_original.txt", "r");
	char line[1000];
	fgets(line, 1000, fp_original);		// size
	fprintf(fp_config, "size=%d\n", num_tasks+5);
	while(fgets(line, 1000, fp_original) != NULL)
		fprintf(fp_config, "%s", line);
	char *task_n;
	int max_y;
	for(int i = 0; i < num_tasks; i++)		// print each task
	{
		task_n = whole_tasks[i]->get_task_name();
		fprintf(fp_config, "\ntitle=%s\n", task_n);
		fprintf(fp_config, "filename=task%d.txt\nlabel=response time\nunit=ms\n", i);
		max_y = whole_tasks[i]->get_period()*1.5;
		fprintf(fp_config, "minY=0\nmaxY=%d\n", max_y/1000);
		fprintf(fp_config, "tickUnit=0.5\nxRange=1.0\ndataNum=1\n");	// tick unit, x range, data num
		fprintf(fp_config, "%s\n", task_n);		// legend
		if(strstr(task_n, "dummy") != NULL)		// if dummy, not visible
			fprintf(fp_config, "visible=0\n");
		else
			fprintf(fp_config, "visible=0\n");
	}
	fclose(fp_original);
	fclose(fp_config);
}

// This function gets the current time from the start of simulation as micro sec.
unsigned long long getcurrenttime()
{
	gettimeofday(&now, NULL);
	return (now.tv_sec-reference_time.tv_sec)*1000000 + (now.tv_usec-reference_time.tv_usec);
}

// This function waits until the current time reaches 'to'
void wait(unsigned long long to)
{
	unsigned long long elapsed;
	while(1)
	{
		elapsed = getcurrenttime();
		if(elapsed > to)
			return;
	}
}

// This is a thread for receiving CAN message.
void* receive_CAN_thread(void *arg)
{
	TPCANMsg msg;
	unsigned long long current_time;
	int unread_count = 0;

	do
	{
		// Check the receive queue for new messages
		//
		errno = CAN_Read(hCAN1, &msg);
		if(!errno)
		{
			unread_count = 0;
			switch(msg.ID)
			{
#include "can_read.hh"
#include "dummy.hh"

				default:
//					printf("not defined msg\n");
					break;
			}
		}
		else		// no messages
		{
			current_time = getcurrenttime();
			while(getcurrenttime() < current_time+1000)
			{
				;
			}
			//
#ifdef DEBUG
			CAN_GetErrorText(result, 9, strMsg);
			cout << strMsg << endl;
#endif
		}

	} while(loop_condition == 1);

	return NULL;
}

/* This function tries to send CAN messages via CAN bus.
 * If there is no message to send at this time, do nothing.
 */
void try_send_data_via_can(FILE *fp)
{
	CAN_Msg *send_data;

	if(waiting_data.empty())
		return;

	if(waiting_data.front()->get_time() < getcurrenttime())
	{
		char strMsg[256];
		pthread_mutex_lock(&section_for_sending);
		send_data = waiting_data.front();
		waiting_data.pop_front();
		pthread_mutex_unlock(&section_for_sending);

		// print for scheduling plot
		char *task_name = send_data->get_task_name();
		if(send_data->get_time()/1000 > recent_time)
		{
			recent_time = send_data->get_time()/1000;
		}
		int is_start = 1;
		fprintf(fp, "%d, msg: %s, %d\n", recent_time, task_name, is_start);
		recent_time++;
		is_start = 0;
		fprintf(fp, "%d, msg: %s, %d\n", recent_time, task_name, is_start);

		// The message is sent using the PCAN-USB
		if (send_data->get_channel() == 1) 
			errno = CAN_Write(hCAN1, &(send_data->msg));
		else 
			errno = CAN_Write(hCAN2, &(send_data->msg));
		if(errno)
		{
			// An error occurred, get a text describing the error and show it
			cout << strMsg << endl;
		}
		// else : success

		delete send_data;
	}
}

/* This function tries to write the current states to files for plotting schedule behaviors.
 */
void try_write_for_schedule_plot(FILE *fp)
{
	Time_plot *aa;

	if(waiting_plot.empty())
		return;

	while(waiting_plot.front()->get_time() < getcurrenttime())
	{
		aa = waiting_plot.front();
		int time = aa->get_time()/1000;
		char *task_name = aa->get_task_name();
		char *resource_name =  aa->get_resource_name();
		int is_start = aa->get_is_start();
		fprintf(fp, "%d, %s: %s, %d\n", time, resource_name, task_name, is_start);
		fflush(fp);
		waiting_plot.pop_front();
		delete aa;
	}
}

/* This function tries to write the current values to files for plotting values of variables.
 */
void try_write_for_line_plot()
{
	if(passed_time > getcurrenttime())
		return;

	unsigned long long temp_time = getcurrenttime();
	int sec = temp_time/1000000;
	int usec = temp_time%1000000;

	passed_time += 100000;		// 100 ms

		fprintf(acc, "%d.%06d %f\n", sec, usec, user_input[ACCELERATION]);
		fprintf(str, "%d.%06d %f\n", sec, usec, user_input[STEER]);
		fprintf(spd, "%d.%06d %f\n", sec, usec, car_output[SPEED]);
		fprintf(rpm, "%d.%06d %f\n", sec, usec, car_output[RPM]);
		fprintf(yaw, "%d.%06d %f\n", sec, usec, car_output[YAW_RATE]);

	fflush(acc);
	fflush(str);
	fflush(spd);
	fflush(rpm);
	fflush(yaw);
}

/* This function tries to write the current response times of tasks to files for plotting response times.
 */
void try_write_for_response_plot()
{
	Time_plot *aa;
	int sec, usec;
	if((!response_plot.empty()) && (response_plot.front()->get_time() < getcurrenttime()))
	{
		aa = response_plot.front();
		response_plot.pop_front();
		int time = aa->get_time();
		int is_start = aa->get_is_start()/1000;
		sec = time/1000000;
		usec = time%1000000;
		fprintf(response_chart[aa->get_task_num()], "%d.%06d %d\n", sec, usec, is_start);
		fflush(response_chart[aa->get_task_num()]);
		delete aa;
	}
}

/* This function adds Time_plot class for a Task class.
 * In this function, all Time_plot class is sorted in time order.
 * This function is used for measuring response times of tasks
 * 
 * <argument>
 * task: target job
 * plot: list for Time_plot class
 */
void add_time_plot(Task *task, list<Time_plot *> *plot)
{
	Time_plot *t = new Time_plot(task->get_completion_time()-task->get_release_time(), task->get_completion_time(), task->get_id());
	if(plot->empty())
	{
		plot->push_back(t);
	}
	else
	{
		int flag = 0;
		list<Time_plot*>::iterator pos;
		for(pos = plot->begin(); pos != plot->end(); pos++)
		{
			if((*pos)->get_time() > t->get_time())
			{
				plot->insert(pos, t);
				flag = 1;
				break;
			}
		}
		if(flag == 0)
			plot->push_back(t);		// push target to the last position
	}
}
