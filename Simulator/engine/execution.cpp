#include "stdafx.h"
#include "task_created.hh"

Execution::Execution()
{
}

Execution::~Execution()
{
}

/* This function gets the next job who will be executed in the simulator at the current time.
 * 
 * <argument>
 * current_time: currtne time
 * tasks: task vector who has all tasks in the simulator
 *
 * <return>
 * Task class who will be executed
 */
Task* Execution::get_next_job(int current_time, vector<Task_info*> *tasks)
{
	int min_deadline = MAX_INT;
	int task_number = -1;
	Task *re = NULL;
	for(unsigned int i = 0; i < tasks->size(); i++)
	{	
		// get a job who has the earliest effective deadline
		if((!(*tasks)[i]->task_instances.empty()) && (*tasks)[i]->task_instances.front()->get_is_waiting() == 1 &&
			(*tasks)[i]->task_instances.front()->get_effective_release_time() <= current_time &&
			(*tasks)[i]->task_instances.front()->get_effective_deadline() <= min_deadline)
		{
			task_number = i;
			min_deadline = (*tasks)[i]->task_instances.front()->get_effective_deadline();
		}
	}

	if(task_number != -1)
	{
		re = (*tasks)[task_number]->task_instances.front();
		(*tasks)[task_number]->task_instances.pop_front();
	}
	else		// if no job is in this time, return a job who has the earliest effective release time
	{
		int min_release = MAX_INT;
		for(unsigned int i = 0; i < tasks->size(); i++)
		{	
			// get a job who has the earliest effective release time
			if((!(*tasks)[i]->task_instances.empty()) && (*tasks)[i]->task_instances.front()->get_is_waiting() == 1 &&
				(*tasks)[i]->task_instances.front()->get_effective_release_time() <= min_release)
			{
				task_number = i;
				min_release = (*tasks)[i]->task_instances.front()->get_effective_release_time();
			}
		}
		if(task_number != -1)
		{
			re = (*tasks)[task_number]->task_instances.front();
			(*tasks)[task_number]->task_instances.pop_front();
		}
	}

	return re;
}
