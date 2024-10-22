#include "task_created.hh"
#include "can_api.h"
#include "data_list.hh"

extern vector<Task*> whole_task_functions;
extern list<CAN_Msg *> waiting_data;

// Task1 (CC1)
Task1::Task1():Task()
{
}

Task1::~Task1()
{
}

void Task1::read()
{
	speed = car_output[SPEED];
}

void Task1::write()
{
	data[0] = accel;
	data_size = 1;
}

void Task1::procedure()
{
	accel = ((60.0 - speed) / 60.0);
}

// Task2 (CC2)
Task2::Task2():Task()
{
}

Task2::~Task2()
{
}

void Task2::read()
{
	accel = whole_task_functions[0]->data[0];
}

void Task2::write()
{
	data[0] = accel;
	data_size = 1;
}

void Task2::procedure()
{
	;
}


// Task3 (LK1)
Task3::Task3():Task()
{
}

Task3::~Task3()
{
}

void Task3::read()
{
	lateral_distance = car_output[LATERAL_DISTANCE];
}

void Task3::write()
{
	data[0] = lateral_distance;
	data_size = 1;
}

void Task3::procedure()
{
	;
}


// Task4 (LK2)
Task4::Task4():Task()
{
}

Task4::~Task4()
{
}

void Task4::read()
{
	lateral_distance = whole_task_functions[2]->data[0];
}

void Task4::write()
{
	data[0] = steering;
	data[1] = lead_distance;
	data_size = 2;
}

void Task4::procedure()
{
	lead_distance = 15.0;
	steering = (-4.0 - lateral_distance) * 18;
}

// Task5 (LK3)
Task5::Task5():Task()
{
}

Task5::~Task5()
{
}

void Task5::read()
{
	steering = whole_task_functions[3]->data[0];
	lead_distance = whole_task_functions[3]->data[1];
}

void Task5::write()
{
	data[0] = steering;
	data[1] = lead_distance;
}

void Task5::procedure()
{
	;
}
