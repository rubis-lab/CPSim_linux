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

#define SHM 1230
#define SHM2 4320

#define TIME_DURATION 3000

#define NUM_MSG 4

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

void* init_shared_mem(int *id, int key, int data_size, int num_of_data, void *mem);
int free_shared_mem(void *data, int id);
void* wait_type_input(void *arg);
void* receive_thread(void *arg);
void send_data_via_CAN(TPCANMsg *msg, float *data1, float *data2);
void do_exit(int error);
void signal_handler(int signal);
void init();

int main(int argc, char *argv[])
{
	init();

	// CAN open
	h = LINUX_CAN_Open(szDevNode, O_RDWR);
	if (!h)
	{
		printf("can't open %s\n", szDevNode);
		return 0;
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
		return 0;
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
			return 0;
		}
	}

	// init msg parameter.
	for(int i = 0; i < num_msg; i++)
	{
		can_msg[i].MSGTYPE = 0;
		can_msg[i].LEN = 8;
	}
	can_msg[0].ID = USER_MSG1;
	can_msg[1].ID = USER_MSG2;
	can_msg[2].ID = START_SIM;
	can_msg[3].ID = STOP_SIM;

	user_input = (float*)init_shared_mem(&shmid_input, SHM, sizeof(float), size_user_input, shared_memory_input);		// shm start (input)
	torcs_output = (float*)init_shared_mem(&shmid_output, SHM2, sizeof(float), size_torcs_output, shared_memory_output);		// shm start (output)

	pthread_t thread[2];
	int re;
	re = pthread_create(&thread[0], NULL, wait_type_input, NULL);		// type input thread
	if(re < 0)
	{
		perror("thread create error : type input");
		exit(0);
	}

	re = pthread_create(&thread[1], NULL, receive_thread, NULL);		// CAN thread
	if(re < 0)
	{
		perror("thread create error : CAN");
		exit(0);
	}

	// main loop
	usleep(1000000);
	gettimeofday(&prev, NULL);
	// send start signal to torcs part PC
	send_data_via_CAN(&can_msg[2], &user_input[0], &user_input[1]);
	while(loop_condition != 2)
	{
		gettimeofday(&cur, NULL);
		if((cur.tv_sec-prev.tv_sec)*1000000+cur.tv_usec-prev.tv_usec < time_duration)
			continue;
		gettimeofday(&prev, NULL);

		// send torcs_output via CAN
		send_data_via_CAN(&can_msg[0], &user_input[0], &user_input[1]);
		send_data_via_CAN(&can_msg[1], &user_input[2], &user_input[3]);
	}

	int status;
	pthread_join(thread[0], (void **)&status);
	pthread_join(thread[1], (void **)&status);

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

void* wait_type_input(void *arg)
{
	while(loop_condition != 2)
	{
		printf("\ntype command (0: pause, 1: start, 2: exit, 3: delay increase, 4: delay recover, 5: steering toggle): ");
		loop_condition = getchar() - '0';
		getchar();
	}

	return (void *)0;
}

void* receive_thread(void *arg)
{
	while(loop_condition != 2)
	{
		if ((errno = CAN_Read(h, &receive_msg))) 
		{
			perror("CAN_Read()");

			// CAN close
			if(h)
				CAN_Close(h);
			exit(0);
		}
		switch(receive_msg.ID)
		{
			case TORCS_MSG1:
				memcpy(&torcs_output[0], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&torcs_output[2], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case TORCS_MSG2:
				memcpy(&torcs_output[1], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&torcs_output[3], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case TORCS_MSG3:
				memcpy(&torcs_output[4], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&torcs_output[5], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case TORCS_MSG4:
				memcpy(&torcs_output[6], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&torcs_output[7], &(receive_msg.DATA[4]), sizeof(float));
				break;

			case TORCS_MSG5:
				memcpy(&torcs_output[8], &(receive_msg.DATA[0]), sizeof(float));
				memcpy(&torcs_output[9], &(receive_msg.DATA[4]), sizeof(float));
				break;

			default:
				printf("not defined msg\n");
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
		if(h)
			CAN_Close(h);
		exit(0);
	}
}

void do_exit(int error)
{
	loop_condition = 2;

	// send stop signal to torcs part PC
//	send_data_via_CAN(&can_msg[3], &user_input[0], &user_input[1]);

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
