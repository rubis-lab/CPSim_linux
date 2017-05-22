#include "stdafx.h"
#include "task_created.hh"

// default constructor for Task_info class
Task_info::Task_info()
{
	scheduler_link = NULL;
	priority = -1;
}

// constructor for Task_info class
Task_info::Task_info(int id_input, int type_input, int period_input, int wcet_input,int modified_input,
	int phase_input, int visible_input, int read_input, int write_input, int dead_input, int pri,
	char *name, Scheduler *scheduler):Component(id_input)
{
	type = type_input;
	period = period_input;
	wcet = wcet_input;
	modified_wcet = modified_input;
	phase = phase_input;
	visible = visible_input;
	is_read = read_input;
	is_write = write_input;
	r_deadline = dead_input;
	priority = pri;
	a_deadline = r_deadline + phase;
	next_release_time = phase;
	next_job_id = 0;
	strcpy(task_name, name);
	scheduler_link = scheduler;
	scheduler->tasks.push_back(this);
}

Task_info::~Task_info()
{
}

// This function returns 'type' variable in Task_info class
int Task_info::get_type()
{
	return type;
}

// This function returns 'period' variable in Task_info class
int Task_info::get_period()
{
	return period;
}

// This function returns 'wcet' variable in Task_info class
int Task_info::get_wcet()
{
	return wcet;
}

// This function returns 'modified_wcet' variable in Task_info class
int Task_info::get_modified_wcet()
{
	return modified_wcet;
}

// This function returns 'phase' variable in Task_info class
int Task_info::get_phase()
{
	return phase;
}

// This function returns 'visible' variable in Task_info class
int Task_info::get_visible()
{
	return visible;
}

// This function returns 'is_read' variable in Task_info class
int Task_info::get_is_read()
{
	return is_read;
}

// This function returns 'is_write' variable in Task_info class
int Task_info::get_is_write()
{
	return is_write;
}

// This function returns 'r_deadline' variable in Task_info class
int Task_info::get_r_deadline()
{
	return r_deadline;
}

// This function returns 'priority' variable in Task_info class
int Task_info::get_priority()
{
	return priority;
}

// This function returns 'a_deadline' variable in Task_info class
int Task_info::get_a_deadline()
{
	return a_deadline;
}

// This function returns 'next_release_time' variable in Task_info class
int Task_info::get_next_release_time()
{
	return next_release_time;
}

// This function returns 'next_job_id' variable in Task_info class
int Task_info::get_next_job_id()
{
	return next_job_id;
}

// This function returns 'task_name' variable in Task_info class
char* Task_info::get_task_name()
{
	return task_name;
}

// This function sets 'visible' variable in Task_info class as 'input'
void Task_info::set_visible(int input)
{
	visible = input;
}

// This function sets 'priority' variable in Task_info class as 'input'
void Task_info::set_priority(int input)
{
	priority = input;
}

// This function sets 'a_deadline' variable in Task_info class as 'input'
void Task_info::set_a_deadline(int input)
{
	a_deadline = input;
}

// This function sets 'next_release_time' variable in Task_info class as 'input'
void Task_info::set_next_release_time(int input)
{
	next_release_time = input;
}

/* This function increases job_id for next job.
 * After a job is created, next job_id and release time should be modified for next job creation.
 */
void Task_info::job_id_increase()
{
	next_job_id++;
	next_release_time += period;
	a_deadline = next_release_time + r_deadline;
}

/* This function finds a job whose id is 'job_id' in 'task_instances' list in this class.
 * 
 * <argument>
 * job_id: an id of job to find
 *
 * <return>
 * Task class whose id is 'job_id'
 */
Task* Task_info::find_task(int job_id)
{
	list<Task*>::iterator pos;
	for(pos = task_instances.begin(); pos != task_instances.end(); pos++)
	{
		if((*pos)->get_job_id() == job_id)
			return (*pos);
	}
	return NULL;
}

// default constructor for Task_info class
Task::Task()
{
	effective_deadline = MAX_INT-1;
	effective_release_time = 0;
	is_waiting = 0;
	task_link = NULL;
}

// constructor for Task_info class
Task::Task(Task_info *task)
{
	id = task->get_id();
	job_id = task->get_next_job_id();
	release_time = task->get_next_release_time();
	a_deadline = release_time + task->get_r_deadline();
	if(task->get_is_write() == 1)
		effective_deadline = a_deadline;
	else
		effective_deadline = MAX_INT-1;

	if(task->get_is_read() == 1)
		effective_release_time = release_time;
	else
		effective_release_time = 0;
	task_link = task;
	is_waiting = 0;
}

// constructor for Task_info class
Task::Task(int task_id, int job_id_):Component(task_id)
{
	job_id = job_id_;
	effective_deadline = MAX_INT-1;
	effective_release_time = 0;
	task_link = NULL;
	is_waiting = 0;
}

// constructor for Task_info class
Task::Task(int task_id, int job_id_, Task_info *task):Component(task_id)
{
	job_id = job_id_;
	task_link = task;
	release_time = task->get_next_release_time();
	a_deadline = release_time + task->get_r_deadline();
	if(task != NULL && task->get_is_write() == 1)
		effective_deadline = a_deadline;
	else
		effective_deadline = MAX_INT-1;

	if(task != NULL && task->get_is_read() == 1)
		effective_release_time = release_time;
	else
		effective_release_time = 0;
	is_waiting = 0;
}

// constructor for Task_info class
Task::Task(int task_id, int job_id_, int release_input, int dead_input, Task_info *task):Component(task_id)
{
	job_id = job_id_;
	task_link = task;
	release_time = release_input;
	a_deadline = dead_input;
	if(task != NULL && task->get_is_write() == 1)
		effective_deadline = a_deadline;
	else
		effective_deadline = MAX_INT-1;

	if(task != NULL && task->get_is_read() == 1)
		effective_release_time = release_time;
	else
		effective_release_time = 0;
	is_waiting = 0;
}

Task::~Task()
{
}

// This function returns 'job_id' variable in Task class
int Task::get_job_id()
{
	return job_id;
}

// This function returns 'release_time' variable in Task class
int Task::get_release_time()
{
	return release_time;
}

// This function returns 'a_deadline' variable in Task class
int Task::get_a_deadline()
{
	return a_deadline;
}

// This function returns 'effective_deadline' variable in Task class
int Task::get_effective_deadline()
{
	return effective_deadline;
}

// This function returns 'effective_release_time' variable in Task class
int Task::get_effective_release_time()
{
	return effective_release_time;
}

// This function returns 'is_waiting' variable in Task class
int Task::get_is_waiting()
{
	return is_waiting;
}

// This function returns 'start_time' variable in Task class
unsigned long long Task::get_start_time()
{
	return start_time;
}

// This function returns 'completion_time' variable in Task class
unsigned long long Task::get_completion_time()
{
	return completion_time;
}

// This function sets 'job_id' variable in Task class as 'input'
void Task::set_job_id(int input)
{
	job_id = input;
}

// This function sets 'release_time' variable in Task class as 'input'
void Task::set_release_time(int input)
{
	release_time = input;
}

// This function sets 'a_deadline' variable in Task class as 'input'
void Task::set_a_deadline(int input)
{
	a_deadline = input;
}

// This function sets 'effective_deadline' variable in Task class as 'input'
void Task::set_effective_deadline(int input)
{
	effective_deadline = input;
}

// This function sets 'effective_release_time' variable in Task class as 'input'
void Task::set_effective_release_time(int input)
{
	effective_release_time = input;
}

// This function sets 'is_waiting' variable as 1
void Task::set_waiting()
{
	is_waiting = 1;
}

// This function sets 'start_time' variable in Task class as 'input'
void Task::set_start_time(unsigned long long input)
{
	start_time = input;
}

// This function sets 'completion_time' variable in Task class as 'input'
void Task::set_completion_time(unsigned long long input)
{
	completion_time = input;
}
