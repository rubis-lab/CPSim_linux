#include "task_created.hh"

// default constructor for Event class
Event::Event()
{
	task_info_link = NULL;
	task_link = NULL;
}

// constructor for Event class
Event::Event(int type_input, int time_input, int remaining_input, Task_info *task_info, Task *task)
{
	type = type_input;
	time = time_input;
	remaining_time = remaining_input;
	task_info_link = task_info;
	task_link = task;
	if(task != NULL)
	{
		task_id = task->get_id();
		job_id = task->get_job_id();
	}
}

Event::~Event()
{
}

// This function returns 'task_id' variable in Event class
int Event::get_task_id()
{
	return task_id;
}

// This function returns 'job_id' variable in Event class
int Event::get_job_id()
{
	return job_id;
}

// This function returns 'type' variable in Event class
int Event::get_type()
{
	return type;
}

// This function returns 'time' variable in Event class
int Event::get_time()
{
	return time;
}

// This function returns 'remaining_time' variable in Event class
int Event::get_remaining_time()
{
	return remaining_time;
}

// This function sets 'task_id' variable in Event class as 'input'
void Event::set_task_id(int input)
{
	task_id = input;
}

// This function sets 'job_id' variable in Event class as 'input'
void Event::set_job_id(int input)
{
	job_id = input;
}

// This function sets 'type' variable in Event class as 'input'
void Event::set_type(int input)
{
	type = input;
}

// This function sets 'time' variable in Event class as 'input'
void Event::set_time(int input)
{
	time = input;
}

// This function sets 'remaining_time' variable in Event class as 'input'
void Event::set_remaining_time(int input)
{
	remaining_time = input;
}
