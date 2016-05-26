#ifndef __CAN_APIH__
#define __CAN_APIH__

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <list>
#include <pcan.h>
#include <libpcan.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

using namespace std;

// for CAN
#define PCANUSB1	"/dev/pcan32"
#define PCANUSB2	"/dev/pcan32"

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
	CAN_Msg(unsigned long long, int, int, float, float, char*);
	~CAN_Msg();
	TPCANMsg msg;					// message struct provided by PCAN-USB API

	unsigned long long get_time();
	int get_channel();
	char* get_task_name();
};

void insert_can_msg(list<CAN_Msg *> *msg_list, CAN_Msg *input);

#endif
