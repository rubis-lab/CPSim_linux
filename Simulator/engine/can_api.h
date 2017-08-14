#ifndef __CAN_APIH__
#define __CAN_APIH__

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <list>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#ifndef NOCANMODE
#include <pcan.h>
#include <libpcan.h>
#endif

using namespace std;

// for CAN
#define PCANUSB1	"/dev/pcan32"
#define PCANUSB2	"/dev/pcan32"

#ifdef NOCANMODE
typedef struct
{
    unsigned int ID;            // 11/29 bit code
    unsigned char MSGTYPE;      // bits of MSGTYPE
    unsigned char LEN;          // count of data bytes (0..8)
    unsigned char DATA[8];      // data bytes, up to 8
} TPCANMsg;
#endif

/* CAN Message class
 * This class is responsible for sending messages via CAN bus
 */
class CAN_Msg
{
private:
	unsigned long long time;		// (expected) sending time
	int channel;					// CAN bus channel
	char task_name[20];				// a task's name who tries to send this message

public:
	CAN_Msg();
	CAN_Msg(unsigned long long, int, int, int, int, int, float, float, char*);
	~CAN_Msg();
	TPCANMsg msg;					// message struct provided by PCAN-USB API
	int num_data;
	int data_index1;
	int data_index2;
	float output_data1;
	float output_data2;

	unsigned long long get_time();
	int get_channel();
	char* get_task_name();
};

void insert_can_msg(list<CAN_Msg *> *msg_list, CAN_Msg *input);

#endif
