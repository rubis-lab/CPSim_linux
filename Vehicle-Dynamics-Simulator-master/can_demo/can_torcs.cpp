#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>    // O_RDWR
#include <libpcan.h>
#include <ctype.h>

#include "can_msg.h"
#include "data_list.h"

#define SHM 1230
#define SHM2 4320

#define TIME_DURATION 2000

#define NUM_MSG 5

// for shared mem (input)
int shmid_input;
float *user_input;
int size_user_input = 10;	// 0: up, 1: down, 2: left, 3: right
void *shared_memory_input = (void *)0;

// for shared mem (output)
int shmid_output;
float *torcs_output;
int size_torcs_output = 200;
void *shared_memory_output = (void *)0;

// for CAN
HANDLE h;
const char *szDevNode = DEFAULT_NODE;
char txt[VERSIONSTRING_LEN];
int nType = HW_PCI;
__u32 dwPort = 0;
__u16 wIrq = 0;
__u16 wBTR0BTR1 = CAN_BAUD_1M;
int nExtended = CAN_INIT_TYPE_ST;
int num_msg = NUM_MSG;
TPCANMsg can_msg[NUM_MSG];
TPCANMsg receive_msg;

// global variables
struct timeval prev, cur;
int loop_condition = 0;
int time_duration = TIME_DURATION;
int is_reconnecting = 0;

// for number of running tasks
int dummy[3] = {0, 0, 0};

void* init_shared_mem(int *id, int key, int data_size, int num_of_data, void *mem);
int free_shared_mem(void *data, int id);
void init_can();
void* receive_thread(void *arg);
void send_data_via_CAN(TPCANMsg *msg, float *data1, float *data2);
void reconnect();
void reexecute_and_exit();
void do_exit(int error);
void signal_handler(int signal);
void init();

int main(int argc, char *argv[])
{
	init();

	// CAN initialize
	init_can();

	user_input = (float*)init_shared_mem(&shmid_input, SHM, sizeof(float), size_user_input, shared_memory_input);		// shm start (input)
	torcs_output = (float*)init_shared_mem(&shmid_output, SHM2, sizeof(float), size_torcs_output, shared_memory_output);		// shm start (output)

	pthread_t thread;
	int re;
	re = pthread_create(&thread, NULL, receive_thread, NULL);		// CAN thread
	if(re < 0)
	{
		perror("thread create error : CAN");
		exit(0);
	}

	// main loop
	usleep(1000000);
	gettimeofday(&prev, NULL);
	while(1)
	{
		gettimeofday(&cur, NULL);
		if((cur.tv_sec-prev.tv_sec)*1000000+cur.tv_usec-prev.tv_usec < time_duration)
			continue;
		gettimeofday(&prev, NULL);

		// send torcs_output via CAN
		send_data_via_CAN(&can_msg[0], &torcs_output[0], &torcs_output[2]);
		send_data_via_CAN(&can_msg[1], &torcs_output[1], &torcs_output[3]);
		send_data_via_CAN(&can_msg[2], &torcs_output[4], &torcs_output[5]);
		send_data_via_CAN(&can_msg[3], &torcs_output[6], &torcs_output[7]);
		send_data_via_CAN(&can_msg[4], &torcs_output[8], &torcs_output[9]);
	}

	int status;
	pthread_join(thread, (void **)&status);

	do_exit(0);

	return 0;
}

void* init_shared_mem(int *id, int key, int data_size, int num_of_data, void *mem)
{
	/***** shm start *******/
	*id = shmget((key_t)key, data_size*num_of_data, 0666|IPC_CREAT);
	if(*id == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	mem = shmat(*id, (void *)0, 0);
	if(mem == (void *)-1)
	{
		perror("shmat failed : ");
		exit(0);
	}
	/***********************/

	return mem;
}

int free_shared_mem(void *data, int id)
{
	shmdt(data);
	return shmctl(id, IPC_RMID, 0);
}

void init_can()
{
	// CAN open
	h = LINUX_CAN_Open(szDevNode, O_RDWR);
	if (!h)
	{
		printf("can't open %s\n", szDevNode);
		exit(1);
	}

	// clear status
	CAN_Status(h);
  
	// get version info
	errno = CAN_VersionInfo(h, txt);
	if (!errno)
		printf("driver version = %s\n", txt);
	else
	{
		perror("CAN_VersionInfo()");
		if(h)
			CAN_Close(h);
		exit(1);
	}
  
	// init to a user defined bit rate
	if (wBTR0BTR1) 
	{
		errno = CAN_Init(h, wBTR0BTR1, nExtended);
		if (errno) 
		{
			perror("CAN_Init()");
			if(h)
				CAN_Close(h);
			exit(1);
		}
	}

	// init msg parameter.
	for(int i = 0; i < num_msg; i++)
	{
		can_msg[i].MSGTYPE = 0;
		can_msg[i].LEN = 8;
	}
	can_msg[0].ID = TORCS_MSG1;
	can_msg[1].ID = TORCS_MSG2;
	can_msg[2].ID = TORCS_MSG3;
	can_msg[3].ID = TORCS_MSG4;
	can_msg[4].ID = TORCS_MSG5;

	printf("CAN init. end\n");
}

void* receive_thread(void *arg)
{
//	pid_t pid1, pid2;
	unsigned int task_num, on_off;
	int exception_number = 0;

	while(loop_condition != 2)
	{
		if ((errno = CAN_Read(h, &receive_msg))) 
		{
			perror("CAN_Read()");

			// CAN close
//			if(h)
//				CAN_Close(h);
//			reexecute_and_exit();
			reconnect();
		}
		switch(receive_msg.ID)
		{
			case USER_MSG1:
				memcpy(&user_input[0], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&user_input[1], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case USER_MSG2:
				memcpy(&user_input[2], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&user_input[3], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case START_SIM:		// launch TORCS
//				pid1 = fork();
//				if(pid1 == 0)
//					execl("/usr/local/bin/torcs", "torcs", (char*) 0);
				break;

			case STOP_SIM:		// stop
//				pid2 = fork();
//				if(pid2 == 0)
//					execl("/home/jckim/stop.sh", "/home/jckim/stop.sh", (char*) 0);
				break;

			case INJECTION:		// task injection
				memcpy(&task_num, &(receive_msg.DATA[0]), sizeof(unsigned int));
				memcpy(&on_off, &(receive_msg.DATA[4]), sizeof(unsigned int));
				if(task_num < 3 || task_num > 5)
					break;
				if(dummy[task_num-3] == 0 && on_off == 1)		// on
				{
					user_input[NUM_DUMMY]++;
					dummy[task_num-3] = 1;
				}
				else if(dummy[task_num-3] == 1 && on_off == 0)		// off
				{
					user_input[NUM_DUMMY]--;
					dummy[task_num-3] = 0;
				}
				break;

			case VISION:
			case LKAS_ECU:
			case LKAS_ACK:
				break;

			default:
				printf("unexpected msg detected!!! ID: %d\n", receive_msg.ID);
				exception_number++;
				if(exception_number > 20)	// try reconnect
				{
					exception_number = 0;
					reconnect();
//					reexecute_and_exit();
				}
				break;
		}
	}

	return (void *)0;
}

void send_data_via_CAN(TPCANMsg *msg, float *data1, float *data2)
{
	memcpy(&(msg->DATA[0]), data1, sizeof(float));
	memcpy(&(msg->DATA[4]), data2, sizeof(float));

	if ((errno = CAN_Write(h, msg)))
	{
		perror("CAN_Write()");

		// CAN close
//		if(h)
//			CAN_Close(h);
//		reexecute_and_exit();
		reconnect();
	}
}

void reconnect()
{
	if(is_reconnecting == 1)	// already reconnecting
	{
		printf("already reconnecting\n");
		usleep(1000000);
		return;
	}
	is_reconnecting = 1;
	printf("reconnecting...\n\n");
	if(h)
	{
		CAN_Close(h);
	}

	usleep(1000000);

	// CAN initialize
	init_can();
	is_reconnecting = 0;
}

void reexecute_and_exit()
{
	pid_t pid1;
	pid1 = fork();
	if(pid1 == 0)
	{
		usleep(3000000);
		printf("exec\n");
		execl("/home/jckim/Dropbox/comm_with_torcs/can_demo/can_torcs", "/home/jckim/Dropbox/comm_with_torcs/can_demo/can_torcs", (char*) 0);
		printf("not reached\n");
	}
	else
		do_exit(1);
}

void do_exit(int error)
{
	loop_condition = 2;

	if(h)
	{
		printf("\nCAN end\n");
		CAN_Close(h);
	}
	// shared memory end.
	free_shared_mem(user_input, shmid_input);		// shm end (input)
	free_shared_mem(torcs_output, shmid_output);	// shm end (output)
	exit(error);
}

void signal_handler(int signal)
{
	do_exit(0);
}

void init()
{
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
}

