#include "components.h"

/* Scheduler */
Scheduler_Universal::Scheduler_Universal()
{
	num_tasks = 0;
}

Scheduler_Universal::Scheduler_Universal(int policy_input, int is_preemptible_input)
{
	num_tasks = 0;
	policy = policy_input;
	is_preemptible = is_preemptible_input;
}

Scheduler_Universal::Scheduler_Universal(int policy_input, int is_preemptible_input, vector<Task_info*> tasks_input, vector<list<Node*> > node_list_input)
{
	policy = policy_input;
	is_preemptible = is_preemptible_input;
	tasks = tasks_input;
	node_list = node_list_input;
	num_tasks = tasks.size();
}

Scheduler_Universal::~Scheduler_Universal()
{
	/*
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		job_list[i].clear();
	}
	delete(job_list);
	*/
}

void Scheduler_Universal::add_task(Task_info *t)
{
	list<Node*> l;
	node_list.push_back(l);
	tasks.push_back(t);
	num_tasks++;
}

void Scheduler_Universal::generate_nodes(int start_time, int end_time)
{
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		int cur_time = tasks[i]->phase;
		int job_id = 0;

		// get the first job after start_time
		while(cur_time < start_time)
		{
			cur_time += tasks[i]->period;
			job_id++;
		}

		// generate jobs from start_time to end_time
		while(cur_time < end_time)
		{
			Node *n = new Node(tasks[i], job_id, cur_time, cur_time+tasks[i]->r_deadline);
			n->actual_execution_time_ECU = rand()%(tasks[i]->wcet_ECU-tasks[i]->bcet_ECU+1) + tasks[i]->bcet_ECU;
			n->remaining_time_ECU = n->actual_execution_time_ECU;
			n->actual_execution_time_PC = n->actual_execution_time_ECU*n->task->modified_rate/100;
			n->remaining_time_PC = n->actual_execution_time_PC;
			node_list[i].push_back(n);
			job_id++;
			cur_time += tasks[i]->period;
		}
	}
}

// standard 0: RM, 1: DM, 2: EDF
void Scheduler_Universal::set_priority(int policy)
{
	int num_completed_node_list = 0;

	// set initial jobs on all job lists
	vector<list<Node*>::iterator> cur_node;
//	cur_job = (list<Job*>::iterator *)malloc(sizeof(list<Job*>::iterator)*num_tasks);
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos = node_list[i].begin();
		cur_node.push_back(pos);
	}

	// set priorities
	int cur_pri = 0;
	while(num_completed_node_list < num_tasks)
	{
		// find target
		int min_value = MAX_INT;
		int min_index = -1;
		for(unsigned int i = 0; i < num_tasks; i++)
		{
			if(cur_node[i] == node_list[i].end())
				continue;
			
			int cur_value = 0;
			switch(policy)
			{
				case 0:			// RM
					cur_value = tasks[i]->period;
					break;

				case 1:			// DM
					cur_value = tasks[i]->r_deadline;
					break;

				case 2:			// EDF
					cur_value = (*cur_node[i])->deadline_ECU;
					break;

				case 3:			// FIFO, FCFS
					cur_value = (*cur_node[i])->release_time;
					break;

				case 4:			// SJF
					cur_value = (*cur_node[i])->actual_execution_time_ECU;
					break;

				default:
					break;
			}
			if(cur_value < min_value)
			{
				min_value = cur_value;
				min_index = i;
			}
		}

		// process
		(*cur_node[min_index])->priority = cur_pri;
		cur_node[min_index]++;
		if(cur_node[min_index] == node_list[min_index].end())
			num_completed_node_list++;
		cur_pri++;
	}
}

// get start and finish times of all jobs
// best_worst: 0 - best, 1 - worst
void Scheduler_Universal::generate_schedule(int best_worst, int is_preemptible)
{
	int pre_time = 0;		// the recent time when a job gets a CPU
	int cur_time = 0;		// the next current time when the job will finish
	int highest_job_index = -1;		// task index of the job who gets a CPU
	int num_completed_job_list = 0;

	// set remaining execution time based on best_worst
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
			{
				if(best_worst == 0)
					(*pos)->remaining_time_ECU = (*pos)->task->bcet_ECU;
				else if(best_worst == 1)
					(*pos)->remaining_time_ECU = (*pos)->task->wcet_ECU;
			}
	}

	// set initial nodes on all job lists
	vector<list<Node*>::iterator> cur_node;
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos = node_list[i].begin();
		cur_node.push_back(pos);
	}

	while(1)
	{
		// get the highest priority job whose release time is earlier than next current time when this is preemptible system
		if(is_preemptible)
		{
			int max_priority = MAX_INT;
			int max_index = -1;

			// get the highest priority node
			for(unsigned int i = 0; i < num_tasks; i++)
			{
				if(cur_node[i] == node_list[i].end())
					continue;
			
				if((*cur_node[i])->release_time < cur_time && (*cur_node[i])->priority < max_priority)
				{
					if((highest_job_index == -1) || ((*cur_node[i])->priority < (*cur_node[highest_job_index])->priority))
					{
						max_priority = (*cur_node[i])->priority;
						max_index = i;
					}
				}
			}

			if(max_index != highest_job_index && max_index != -1 && highest_job_index != -1)		// preempted
			{
				cur_time = (*cur_node[max_index])->release_time;
				(*cur_node[highest_job_index])->remaining_time_ECU -= (cur_time-pre_time);	// adjust the preempted job's remaining time
				
				// new job
				highest_job_index = max_index;								// job change
				if(best_worst == 0 && (*cur_node[highest_job_index])->min_start_time_ECU == -1)			// first start for min
					(*cur_node[highest_job_index])->min_start_time_ECU = cur_time;
				else if(best_worst == 1 && (*cur_node[highest_job_index])->max_start_time_ECU == -1)			// first start for min
					(*cur_node[highest_job_index])->max_start_time_ECU = cur_time;
				pre_time = cur_time;										// record this time
				cur_time += (*cur_node[highest_job_index])->remaining_time_ECU;	// update the next current time

				continue;
			}
		}

		// the current job can finish
		if(highest_job_index != -1)
		{
			if(best_worst == 0)
				(*cur_node[highest_job_index])->min_finish_time_ECU = cur_time;
			else
				(*cur_node[highest_job_index])->max_finish_time_ECU = cur_time;
			(*cur_node[highest_job_index])->remaining_time_ECU = 0;
			cur_node[highest_job_index]++;
			if(cur_node[highest_job_index] == node_list[highest_job_index].end())
				num_completed_job_list++;
			if(num_completed_job_list == num_tasks)
				break;
			highest_job_index = -1;
		}

		// select next job before the next current time
		int max_priority = MAX_INT;
		int max_index = -1;
		for(unsigned int i = 0; i < num_tasks; i++)
		{
			if(cur_node[i] == node_list[i].end())
				continue;
			
			if((*cur_node[i])->release_time <= cur_time && (*cur_node[i])->priority < max_priority)
			{
				max_priority = (*cur_node[i])->priority;
				max_index = i;
			}
		}

		// if there is no job, get the earliest released job
		if(max_index == -1)
		{
			int min_release_time = MAX_INT;
			for(unsigned int i = 0; i < num_tasks; i++)
			{
				if(cur_node[i] == node_list[i].end())
					continue;
			
				if(((*cur_node[i])->release_time < min_release_time) || ((*cur_node[i])->release_time == min_release_time && (*cur_node[i])->priority < max_priority))
				{
					min_release_time = (*cur_node[i])->release_time;
					max_priority = (*cur_node[i])->priority;
					max_index = i;
				}
			}
		}

		// start this job
		highest_job_index = max_index;
		if((*cur_node[highest_job_index])->release_time > cur_time)		// current time update
			cur_time = (*cur_node[highest_job_index])->release_time;
		pre_time = cur_time;
		cur_time += (*cur_node[highest_job_index])->remaining_time_ECU;
		if(best_worst == 0 && (*cur_node[highest_job_index])->min_start_time_ECU == -1)
			(*cur_node[highest_job_index])->min_start_time_ECU = pre_time;
		else if(best_worst == 1 && (*cur_node[highest_job_index])->max_start_time_ECU == -1)
			(*cur_node[highest_job_index])->max_start_time_ECU = pre_time;
	}
}

int Scheduler_Universal::get_WCBP_start(Node *target)
{
	// get busy duration for the target job
	int busy_start_time;
	Node *cur_node = target;
	while(1)
	{
		busy_start_time = cur_node->max_start_time_ECU;
		if(cur_node->max_start_time_ECU == cur_node->release_time)		// the first job
			break;

		// get a job(cur_node) whose finish time is same as the current start time(busy_start_time)
		int is_found = 0;
		for(unsigned int i = 0; i < num_tasks; i++)
		{
			list<Node*>::iterator pos;
			for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
			{
				if(((*pos)->priority < target->priority) && ((*pos)->max_finish_time_ECU == busy_start_time))
				{
					is_found = 1;
					cur_node = (*pos);
					break;
				}
			}

			if(is_found == 1)
				break;
		}
	}
	return busy_start_time;
}

void Scheduler_Universal::get_S_set(Node *target)
{
	if(target->min_start_time_ECU == target->max_start_time_ECU)
		return;

	int WCBP_start = get_WCBP_start(target);

	// include jobs into S-set
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
		{
			if((*pos)->is_virtual > 0)
				continue;

			if(((*pos)->priority < target->priority) && ((*pos)->release_time >= WCBP_start)
				&& ((*pos)->release_time < target->max_start_time_ECU))
			{
				target->S_set.push_back((*pos));
			}
		}
	}
}

void Scheduler_Universal::get_F_set(Node *target)
{
	if(target->min_start_time_ECU == target->max_start_time_ECU)
	{
		target->F_set.push_back(target);
		return;
	}

	int WCBP_start = get_WCBP_start(target);

	// include jobs into F-set
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
		{
			if((*pos)->is_virtual > 0)
				continue;

			if(((*pos)->priority < target->priority) && ((*pos)->release_time >= WCBP_start)
				&& ((*pos)->release_time < target->max_finish_time_ECU))
			{
				target->F_set.push_back((*pos));
			}
		}
	}
	target->F_set.push_back(target);	// include target itself
}

void Scheduler_Universal::do_schedule_initial(int start_time, int end_time)
{
	generate_nodes(start_time, end_time);

	// get min, max start and finish time
	set_priority(policy);
	generate_schedule(0, is_preemptible);
	generate_schedule(1, is_preemptible);

	// get S, F-set
	for(unsigned int i = 0; i < num_tasks; i++)
	{
		list<Node*>::iterator pos;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
		{
			get_S_set(*pos);
			get_F_set(*pos);
		}
	}
}

/*
// execution mode 0: bcet, 1: wcet, 2: random, 3: universal (do not generate job list)
void Scheduler_Universal::do_schedule(int start_time, int end_time, int execution_mode)
{
	if(execution_mode < 3)
		generate_jobs(start_time, end_time, execution_mode);
	set_priority(policy);
	generate_schedule(is_preemptible);
}
*/
