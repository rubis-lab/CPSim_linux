#include "can_api.h"

pthread_mutex_t section_for_can_sending = PTHREAD_MUTEX_INITIALIZER;

CAN_Msg::CAN_Msg()
{
}

// constructor for CAN_Msg class
CAN_Msg::CAN_Msg(unsigned long long time_input, int channel_input, int id_input, float data1, float data2, char* name)
{
	time = time_input;
	channel = channel_input;
	memcpy(&(msg.DATA[0]), &data1, sizeof(float));
	memcpy(&(msg.DATA[4]), &data2, sizeof(float));
	msg.ID = id_input;
	msg.MSGTYPE = 0;
	msg.LEN = 8;
	strcpy(task_name, name);
}

CAN_Msg::~CAN_Msg()
{
}

// This function returns 'time' variable in CAN_Msg class
unsigned long long CAN_Msg::get_time()
{
	return time;
}

// This function returns 'channel' variable in CAN_Msg class
int CAN_Msg::get_channel()
{
	return channel;
}

// This function returns 'task_name' variable in CAN_Msg class
char* CAN_Msg::get_task_name()
{
	return task_name;
}

/* This function inserts CAN_Msg class into 'msg_list' list.
 * Each class is inserted into the list according to its time.
 */
void insert_can_msg(list<CAN_Msg *> *msg_list, CAN_Msg *input)
{
	// first element
	if(msg_list->empty())
	{
		pthread_mutex_lock(&section_for_can_sending);
		msg_list->push_back(input);
		pthread_mutex_unlock(&section_for_can_sending);
		return;
	}

	pthread_mutex_lock(&section_for_can_sending);
	list<CAN_Msg*>::iterator pos;
	for(pos = msg_list->begin(); pos != msg_list->end(); pos++)
	{
		if((*pos)->get_time() > input->get_time())
		{
			msg_list->insert(pos, input);
			pthread_mutex_unlock(&section_for_can_sending);
			return;
		}
	}
	msg_list->push_back(input);		// push target to the last position
	pthread_mutex_unlock(&section_for_can_sending);
}
