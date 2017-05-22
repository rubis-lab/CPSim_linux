#include "stdafx.h"
#include "task_created.hh"
#include "can_api.h"
#include "data_list.hh"
#include <sys/time.h>
#include <unistd.h>

#define TIME_WINDOW	1000000		// us
#define OVER_TIME 1000000		// us
#define HYPER 1;		// number of hyperperiod

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
DAG *dag;
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

// for infinite looping
int loop_condition = 1;

// for CAN
HANDLE hCAN1;

void initialize();
void init_CAN(int num_channel);
unsigned long long getcurrenttime();
void wait(unsigned long long to);
void* receive_CAN_thread(void *arg);
void try_send_data_via_can(FILE *fp);
void try_write_for_schedule_plot(FILE *fp);
void try_write_for_line_plot();
void try_write_for_response_plot();
void program_complete();
void add_time_plot(Task *task, list<Time_plot *> *plot);
int calculate_hyper_period(unsigned int start_index, unsigned int size);

// main function
int main(int argc, char* argv[])
{
	// create resources, schedulers, tasks and set parameters
	initialize();

	// set CAN connection for one channel
	init_CAN(1);

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
	while(!execution->ready_queue.empty())
	{
		int next_release_of_executed_job = 0;

		// pick the first node
		// (1) pick a job in the simulation ready queue
		Node *cur_node = get_the_first_node();

		if(cur_node == NULL)	// no job. then jump
		{
			int min_release = get_nearest_start_time();			// get the nearest start time
			cur_time = min_release;			// set cur_time to min_release
			continue;
		}

		// first start of this job
		if(cur_node->remaining_time_PC == cur_node->actual_execution_time_PC)		// start time of this node
			cur_node->start_time_PC = cur_time;

		// get start time of a job who can delay cur_node
		int next_time = get_nearest_start_time();

		if(cur_node->remaining_time_PC <= (next_time-cur_time))		// this node can complete
		{
			cur_time += cur_node->remaining_time_PC;	// cur_time increases
			cur_node->remaining_time_PC = 0;
			cur_node->actual_execution_time_ECU = cur_node->actual_execution_time_PC*100/cur_node->task->modified_rate;
			cur_node->is_executed = 1;
			cur_node->finish_time_PC = cur_time;		// finish time update
			next_release_of_executed_job = cur_node->release_time + dag->hyper_period;

			// simulatability check
			if(cur_node->is_virtual == 1 && cur_node->finish_time_PC > cur_node->min_finish_time_ECU)
				return 0;	// not simulatable

			// pop this job on the ready queue
			ready_queue.remove(cur_node);

			// clear jobs whose deadline would be changed
			dag->deadline_updatable.clear();

			// gather who can be in ready queue after cur_node is executed
			list<Node*> ready_candidates;
			list<Node*>::iterator pos;
			for(pos = cur_node->successors.begin(); pos != cur_node->successors.end(); pos++)
				ready_candidates.push_back(*pos);
			for(pos = cur_node->non_deterministic_successors.begin(); pos != cur_node->non_deterministic_successors.end(); pos++)
				ready_candidates.push_back(*pos);
			

			// kswe. (2) add a now job into OJPG
			if(next_release_of_executed_job >= end_time)
				dag->pop_and_push_node(cur_node, 0);
			else
				dag->pop_and_push_node(cur_node);
			
			// kswe. (3) update start, finish times
			update_start_finish_time(cur_node);

			// kswe. (4) adjust non-determinism
			for(pos = dag->link_updatable.begin(); pos != dag->link_updatable.end(); pos++)
				adjust_non_deterministic(*pos);

			// kswe. (5) update effective deadlines
			update_deadlines_optimized();

			// update ready queue
			for(pos = ready_candidates.begin(); pos != ready_candidates.end(); pos++)
			{
				if((*pos)->predecessors.empty())
					ready_queue.push_back(*pos);
			}
		}
		else
		{
			cur_node->remaining_time_PC -= (next_time-cur_time);	// remaining time decreases
			cur_time += (next_time-cur_time);					// cur_time increases
		}
	}

	// main loop
	while(loop_condition == 1)
	{
		// try sending data
		try_send_data_via_can(fp_chart);

		int current_time_temp = current_time;

		// increase simulation time
		current_time += TIME_WINDOW;

		while(loop_condition == 1)
		{
			// try sending data
			try_send_data_via_can(fp_chart);

			// get next executable job
			next = execution->get_next_job(current_time_temp, &whole_tasks);
			if(next == NULL)
				break;

			if(next->task_link->get_is_read() == 1)		// wait for time synchronization
			{
				do
				{
					try_send_data_via_can(fp_chart);
				} while(next->get_effective_release_time() > getcurrenttime());
			}

			// calculate expected start and end time
			actual_start_time = current_time_temp;
			expected_end_time = actual_start_time + next->task_link->get_modified_wcet();

			// jump simulation time into this job's release time
			if(next->task_link->get_is_read() == 1 && current_time_temp < next->get_effective_release_time())
				current_time_temp = next->get_effective_release_time() + next->task_link->get_modified_wcet();
			else
				current_time_temp += next->task_link->get_modified_wcet();

			// execute a task (job)
			next->read();
			next->procedure();
			next->write();

			delete next;

			// wait during modified wcet
			do
			{
				try_send_data_via_can(fp_chart);
			} while(expected_end_time > getcurrenttime());
		}
	}

	// uninitialize phase
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

	int hyper_period = calculate_hyper_period(0, whole_tasks.size());

	// generate nodes' list for two hyper_period
	// get S-set, F-set for all jobs
	for(unsigned int i = 0; i < resources.size(); i++)
		resources[i]->scheduler_link->do_schedule_initial(0, 2*hyper_period);

	// generate offline guider
	dag = new DAG(whole_tasks, resources, hyper_period);
	dag->generate_offline_guider();

	// generate the initial OJPG
	dag->generate_initial_OJPG();

	// ready for plan and execution
	int target_period = hyper_period * HYPER;
	execution = new Execution(target_period, dag, resources);
	execution->update_deadline();

	// set initial jobs in the ready queue
	for(int i = 0; i < num_tasks; i++)
	{
		if(dag->OJPG[i].front()->predecessors.empty())
			execution->ready_queue.push_back(dag->OJPG[i].front());
	}

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

int calculate_hyper_period(unsigned int start_index, unsigned int size)
{
	if(size <= 0)
		return 0;
	int lcm = whole_tasks[start_index]->period/1000;
	for(unsigned int i = start_index+1; i < start_index+size; i++)
	{
		int temp1 = lcm;
		int temp2 = whole_tasks[i]->period/1000;
		int num1 = temp1;
		int num2 = temp2;
		while(num1 != num2)
		{
			if(num1 > num2)
				num1 = num1 - num2;
			else
				num2 = num2 - num1;
		}
		lcm = (temp1*temp2)/num1;
	}
	return lcm*1000;
}

