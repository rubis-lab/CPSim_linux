#include "stdafx.h"
#include "task_created.hh"
#include "can_api.h"
#include "data_list.hh"
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

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
vector<Task*> whole_task_functions;
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

// for execution time update
vector<unsigned long long> task_start_times;
vector<unsigned long long> task_finish_times;
list<Task_info*> running_tasks;

// for checking thread priority
vector<int> thread_priority;

// for infinite looping
int loop_condition = 1;

// for CAN
HANDLE hCAN1;

void initialize();
void init_CAN();
unsigned long long getcurrenttime();
void wait(unsigned long long to);
void* task_thread(void *arg);
void* receive_CAN_thread(void *arg);
void try_send_data_via_can();
void program_complete();
int calculate_hyper_period(unsigned int start_index, unsigned int size);

// main function
int main(int argc, char* argv[])
{
	// create resources, schedulers, tasks and set parameters
	initialize();

	// set CAN connection for one channel
	init_CAN(1);

	// initialize thread atrributes
    struct sched_param schedParam;
    pthread_t taskThread;
    pthread_attr_t attr;
	cpu_set_t cpu;

    // initialize the thread attributes
    if (pthread_attr_init(&attr)) {
            ERR("Failed to initialize thread attrs\n");
            cleanup(EXIT_FAILURE);
    }
	// set affinity
	CPU_ZERO(&cpu);
	CPU_SET(3, &cpu);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu);
    // force the thread to use custom scheduling attributes
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
            ERR("Failed to set schedule inheritance attribute\n");
            cleanup(EXIT_FAILURE);
    }
    // set the thread to be fifo real time scheduled */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
            ERR("Failed to set FIFO scheduling policy\n");
            cleanup(EXIT_FAILURE);
    }

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
		execution->cur_time = getcurrenttime();
		Node *cur_node = execution->get_the_first_node();

		// execute the job if its deadline is enough early
		list<Node*>::iterator pos;
		if(cur_node != NULL)
		{
			int min_deadline = MAX_INT;
			int max_priority = 50;		// default priority
			for(pos = running_tasks.begin(); pos != running_tasks.end(); pos++)
			{
				if((*pos)->effective_deadline < min_deadline)
				{
					min_deadline = (*pos)->effective_deadline;
					max_priority = thread_priority[(*pos)->task->id];
				}
			}

			if(min_deadline > cur_node->effective_deadline)		// this job can start
			{
				int running_task_id = cur_node->task->id;
				task_finish_times[running_task_id] = 0;
				running_tasks.push_back(cur_node->task);

				// set the thread priority */
				int prio = max_priority+1;;
				schedParam.sched_priority = prio;
				thread_priority[running_task_id] = prio;
				if (pthread_attr_setschedparam(&attr, &schedParam))
				{
						ERR("Failed to set scheduler parameters\n");
						cleanup(EXIT_FAILURE);
				}

				// create task thread
				if (pthread_create(&taskThread, &attr, task_thread, (void*)&running_task_num))
				{
						ERR("Failed to create task thread\n");
						cleanup(EXIT_FAILURE);
				}
			}
		}

		// check running task is complete
		cur_node = NULL;
		for(pos = running_tasks.begin(); pos != running_tasks.end(); pos++)
		{
			if(task_finish_times[(*pos)->task->id] > 0)		// complete
			{
				cur_node = (*pos);

				thread_priority[cur_node->task->id] = 0;
				
				// get actual execution times
				int task_id = (*pos)->task->id;
				int execution_time = task_finish_times[task_id] - task_start_times[task_id];
				for(int i = 0; i < whole_tasks.size(); i++)
				{
					if(task_start_times[i] < task_finish_times[i] && task_start_times[i] > task_start_times[task_id] && task_finish_times[i] < task_finish_times[task_id])	// this job is preempted
						execution_time -= (task_finish_times[i] - task_start_times[i]);
				}

				if(execution_time < cur_node->task->bcet_ECU)
					execution_time = cur_node->task->bcet_ECU;
				else if(execution_time > cur_node->task->wcet_ECU)
					execution_time = cur_node->task->wcet_ECU;

				// set start, execution, finish time
				cur_node->start_time_PC = task_start_times[task_id];
				cur_node->actual_execution_time_PC = execution_time;
				cur_node->finish_time_PC = task_finish_times[task_id];
				running_tasks.remove(cur_node);

				break;
			}
		}

		if(cur_node != NULL)		// this node can complete
		{
			cur_time += cur_node->remaining_time_PC;	// cur_time increases
			cur_node->remaining_time_PC = 0;
			cur_node->actual_execution_time_ECU = cur_node->actual_execution_time_PC*100/cur_node->task->modified_rate;
			cur_node->is_executed = 1;
			next_release_of_executed_job = cur_node->release_time + dag->hyper_period;

			// pop this job on the ready queue
			ready_queue.remove(cur_node);

			// clear jobs whose deadline would be changed
			dag->deadline_updatable.clear();

			// gather who can be in ready queue after cur_node is executed
			list<Node*> ready_candidates;
			for(pos = cur_node->successors.begin(); pos != cur_node->successors.end(); pos++)
				ready_candidates.push_back(*pos);
			for(pos = cur_node->non_deterministic_successors.begin(); pos != cur_node->non_deterministic_successors.end(); pos++)
				ready_candidates.push_back(*pos);

			// (2) add a now job into OJPG
			if(next_release_of_executed_job >= end_time)
				dag->pop_and_push_node(cur_node, 0);
			else
				dag->pop_and_push_node(cur_node);
			
			// (3) update start, finish times
			execution->update_start_finish_time(cur_node);

			// (4) adjust non-determinism
			for(pos = dag->link_updatable.begin(); pos != dag->link_updatable.end(); pos++)
				execution->adjust_non_deterministic(*pos);

			// (5) update effective deadlines
			execution->update_deadlines_optimized();

			// update ready queue
			for(pos = ready_candidates.begin(); pos != ready_candidates.end(); pos++)
			{
				if((*pos)->predecessors.empty())
					ready_queue.push_back(*pos);
			}
		}
		// try sending data
		try_send_data_via_can();
	}

	// uninitialize phase
	CAN_Close(hCAN1);

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

	for(unsigned int i = 0; i < num_tasks; i++)
	{
		task_start_times.push_back(0);
		task_finish_times.push_back(0);
		thread_priority.push_back(0);
	}

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
void init_CAN()
{
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

// This is a thread for executing a task
void* task_thread(void *arg)
{
	// set affinity
	unsigned long mask = 3;

	int task_id = *(int*)arg;

	// read data
	whole_task_functions[task_id]->read();

	// check the start time
	task_start_times[task_id] = getcurrenttime();

	// execute task
	whole_task_functions[task_id]->procedure();

	// check the finish time
	task_finish_times[task_id] = getcurrenttime();

	// write data
	whole_task_functions[task_id]->write();
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
		if(errno == PCAN_ERROR_OK)
		{
			unread_count = 0;
			switch(msg.ID)
			{
#include "can_read.hh"

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
void try_send_data_via_can()
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

		// The message is sent using the PCAN-USB
		errno = CAN_Write(hCAN1, &(send_data->msg));
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

