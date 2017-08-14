#include "task_created.hh"

Plan::Plan()
{
}

Plan::~Plan()
{
}

/* This function sets effective release times of all jobs in the simulator. (ATTENTION: not only jobs in the scheduler but also all jobs in the simulator)
 * 
 * <argument>
 * resources: resource vector who has all resources in the simulator
 * tasks: task vector who has all tasks in the simulator
 */
void Plan::set_effective_release_time(vector<Resource*> *resources, vector<Task_info*> *tasks)
{
	unsigned int num_resources = (unsigned int)resources->size();
	unsigned int num_tasks = (unsigned int)tasks->size();

	// set prev and next job ids (for setting task links)
	vector<int> next_release;
	vector<int> created_release;
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		if((*tasks)[i]->task_instances.empty())
		{
			next_release.push_back(-1);
			created_release.push_back(-1);
		}
		else
		{
			next_release.push_back((*tasks)[i]->task_instances.front()->get_job_id());
			created_release.push_back((*tasks)[i]->task_instances.front()->get_job_id()-1);
		}
	}

	int min_time;
	int resource_number;
	Event *temp_event = NULL;
	while(1)
	{
		min_time = MAX_INT;
		resource_number = -1;

		// get first event
		for(unsigned int i = 0; i < num_resources; i++)
		{
			Scheduler *temp_s = (*resources)[i]->scheduler_link;
			if(!(temp_s->log2.empty()) && (temp_s->log2.front()->get_time() < min_time ||
				(temp_s->log2.front()->get_time() == min_time && temp_s->log2.front()->get_type() == COMPLETE)))
			{
				min_time = temp_s->log2.front()->get_time();
				resource_number = i;
			}
		}
		if(resource_number == -1)		// all event logs are empty
			break;

		temp_event = (*resources)[resource_number]->scheduler_link->log2.front();
		(*resources)[resource_number]->scheduler_link->log2.pop_front();

		if(temp_event->get_type() == START)
		{
			next_release[temp_event->get_task_id()] = temp_event->get_job_id() + 1;		// set next job id

			// set predecessors
			for(unsigned int i = 0; i < temp_event->task_info_link->predecessors.size(); i++)
			{
				int task_number = temp_event->task_info_link->predecessors[i]->get_id();
				int job_number = created_release[task_number];		// predecessors job number
				Task *pred_task = (*tasks)[task_number]->find_task(job_number);
				if(pred_task != NULL)
					temp_event->task_link->predecessors.push_back(pred_task);		// set this job's predecessors
			}

			int max_time;
			if(temp_event->task_info_link->get_is_read() == 1)
				max_time = temp_event->get_time();
			else
				max_time = 0;

			// consider the previous job
			Task *prev = temp_event->task_info_link->find_task(temp_event->get_job_id() - 1);	// get the previous job
			if(prev != NULL && (prev->get_effective_release_time() + temp_event->task_info_link->get_modified_wcet()) > max_time)
				max_time = prev->get_effective_release_time() + temp_event->task_info_link->get_modified_wcet();

			// consider predecessors
			for(unsigned int i = 0; i < temp_event->task_link->predecessors.size(); i++)
			{
				if(temp_event->task_link->predecessors[i]->get_effective_release_time() + 
					temp_event->task_link->predecessors[i]->task_link->get_modified_wcet() > max_time)
					max_time = temp_event->task_link->predecessors[i]->get_effective_release_time() + 
					temp_event->task_link->predecessors[i]->task_link->get_modified_wcet();
			}
			temp_event->task_link->set_effective_release_time(max_time);
			temp_event->task_link->set_waiting();		// set this task as waiting for execution
			delete temp_event;
		}
		else		// complete event
		{
			created_release[temp_event->get_task_id()] = temp_event->get_job_id();		// set created job id

			// set successors
			for(unsigned int i = 0; i < temp_event->task_info_link->successors.size(); i++)
			{
				int task_number = temp_event->task_info_link->successors[i]->get_id();
				int job_number = next_release[task_number];	// successors job number
				Task *suc_task = (*tasks)[task_number]->find_task(job_number);
				if(suc_task != NULL)
					temp_event->task_link->successors.push_back(suc_task);		// set this job's successor
			}

			// add this complete event to log3 for setting effective deadline
			(*resources)[resource_number]->scheduler_link->log3.push_back(temp_event);
		}
	}
}

/* This function sets effective deadlines of all jobs in the simulator. (ATTENTION: not only jobs in the scheduler but also all jobs in the simulator)
 * 
 * <argument>
 * resources: resource vector who has all resources in the simulator
 */
void Plan::set_effective_deadline(vector<Resource*> *resources)
{
	unsigned int num_resources = (unsigned int)resources->size();
	int max_time;
	int resource_number;
	Event *temp_event;
	while(1)
	{
		max_time = -1;
		resource_number = -1;

		// get latest event
		for(unsigned int i = 0; i < num_resources; i++)
		{
			Scheduler *temp_s = (*resources)[i]->scheduler_link;
			if(!(temp_s->log3.empty()) &&
				(temp_s->log3.back()->get_time() > max_time ||
				(temp_s->log3.back()->get_time() == max_time && temp_s->log3.back()->get_type() == COMPLETE)))
			{
				max_time = temp_s->log3.back()->get_time();
				resource_number = i;
			}
		}
		if(resource_number == -1)		// all event logs are empty
			break;

		temp_event = (*resources)[resource_number]->scheduler_link->log3.back();
		(*resources)[resource_number]->scheduler_link->log3.pop_back();
		
		// setting effective deadline
		int min_time;
		if(temp_event->task_info_link->get_is_write() == 1)		// write sync task
			min_time = temp_event->get_time();
		else
			min_time = MAX_INT-1;

		// consider the next job
		Task *next = temp_event->task_info_link->find_task(temp_event->get_job_id() + 1);	// get the next job
		if(next != NULL && (next->get_effective_deadline() - temp_event->task_info_link->get_modified_wcet()) < min_time)
			min_time = next->get_effective_deadline() - temp_event->task_info_link->get_modified_wcet();

		// consider successors
		for(unsigned int i = 0; i < temp_event->task_link->successors.size(); i++)
		{
			if(temp_event->task_link->successors[i]->get_effective_deadline() -
				temp_event->task_link->successors[i]->task_link->get_modified_wcet() < min_time)
				min_time = temp_event->task_link->successors[i]->get_effective_deadline() -
				temp_event->task_link->successors[i]->task_link->get_modified_wcet();
		}
		temp_event->task_link->set_effective_deadline(min_time);
		delete temp_event;
	}
}

/* This function generates an execution plan with jobs after setting effective release times and deadlines of them.
 * 
 * <argument>
 * resources: resource vector who has all resources in the simulator
 * tasks: task vector who has all tasks in the simulator
 */
void Plan::generate_plan(vector<Resource*> *resources, vector<Task_info*> *tasks)
{
	set_effective_release_time(resources, tasks);
	set_effective_deadline(resources);
}
