#include "task_created.hh"
#include "can_api.h"
#include "data_list.hh"

extern list<CAN_Msg *> waiting_data;

// Task0 (LKAS)
Task0::Task0(Task_info *task_info):Task(task_info)
{
}

Task0::~Task0()
{
}

void Task0::read()
{
	;
}

void Task0::write()
{
	user_input_internal[STEER] = steering;

	// can send
	CAN_Msg *can_msg = new CAN_Msg(completion_time, 1, 0x7FE, 1, STEER, STEER, steering, steering, this->task_link->get_task_name());
	insert_can_msg(&waiting_data, can_msg);
}

void Task0::procedure()
{
	// LKAS
	float SC = 1.0;
	float track_angle = car_output[TRACK_ANGLE];
	float angle = track_angle - car_output[YAW];
	float pi = 3.141592;
	while(angle > pi)
	    angle -= 2*pi;
	while(angle < -pi)
	    angle += 2*pi;
	angle -= SC*car_output[DISTANCE]/car_output[TRACK_WIDTH];	// Road Keeping Assist System
    steering = angle;
}


// Task1 (dummy1)
Task1::Task1(Task_info *task_info):Task(task_info)
{
}

Task1::~Task1()
{
}

void Task1::read()
{
	;
}

void Task1::write()
{
	;
}

void Task1::procedure()
{
	;
}


// Task2 (dummy2)
Task2::Task2(Task_info *task_info):Task(task_info)
{
}

Task2::~Task2()
{
}

void Task2::read()
{
	;
}

void Task2::write()
{
	;
}

void Task2::procedure()
{
	;
}


// Task3 (CC1)
Task3::Task3(Task_info *task_info):Task(task_info)
{
}

Task3::~Task3()
{
}

void Task3::read()
{
    ;
}

void Task3::write()
{
	memory_buffer[0] = speed;
	for(unsigned int i = 0; i < successors.size(); i++)
    {
		((Task4*)successors[i])->speed = speed;
    }
}

void Task3::procedure()
{
    speed = car_output[SPEED];
}


// Task4 (CC2)
Task4::Task4(Task_info *task_info):Task(task_info)
{
}

Task4::~Task4()
{
}

void Task4::read()
{
	speed = memory_buffer[0];
}

void Task4::write()
{
	user_input_internal[ACCEL] = accel;
	user_input_internal[BRAKE] = brake;

	// can send
	CAN_Msg *can_msg = new CAN_Msg(completion_time, 1, 0x7FF, 2, ACCEL, BRAKE, accel, accel, this->task_link->get_task_name());
	insert_can_msg(&waiting_data, can_msg);
}

void Task4::procedure()
{
    brake = 0.0;
    if(speed < 10.0)
        accel = 1;
    else if(speed < 60.0)
        accel = 0.5;
    else if(speed < 80.0)
        accel = 0.2;
    else
        accel = 0.0;
}


