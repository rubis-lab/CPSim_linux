#include "components.h"

Execution::Execution(int e_time, DAG *d, vector<Resource*> r)
{
	end_time = e_time;
	dag = d;
	cur_time = 0;
	resources = r;
}

// set target's effective deadline
int Execution::update_deadline_for_each_node(Node *target)
{
	int is_updated = 0;
	int temp_deadline;

	// initial deadline
	if(target->is_virtual > 0)
		temp_deadline = target->min_finish_time_ECU;
	else
		temp_deadline = MAX_INT-1;

	list<Node*>::iterator pos;
	for(pos = target->successors.begin(); pos != target->successors.end(); pos++)
	{
		int deadline = (*pos)->effective_deadline;
		if(deadline < temp_deadline)
			temp_deadline = deadline;
	}
	if(temp_deadline != target->effective_deadline)
	{
		target->effective_deadline = temp_deadline;
		is_updated = 1;
	}
	return is_updated;
}

// check all successors' deadlines are updated
int Execution::all_successor_updated(Node *target)
{
	list<Node*>::iterator pos;
	for(pos = target->successors.begin(); pos != target->successors.end(); pos++)
	{
		if((*pos)->is_deadline_updated == 0)
			return 0;
	}
	return 1;
}

void Execution::update_deadlines_optimized()
{
	while(!dag->deadline_updatable.empty())
	{
		Node *target = dag->deadline_updatable.front();
		dag->deadline_updatable.pop_front();
		int updated = update_deadline_for_each_node(target);
		if(updated == 1 && (!target->predecessors.empty()))
		{
			list<Node*>::iterator pos;
			for(pos = target->predecessors.begin(); pos != target->predecessors.end(); pos++)
				dag->push_job_for_update_deadline(*pos);
		}
	}
}

void Execution::update_deadlines_revised()
{
	unsigned int task_num = dag->task_list.size();
	// initialize updated_deadline
	for(unsigned int i = 0; i < task_num; i++)
	{
		list<Node*>::iterator pos;
		for(pos = dag->OJPG[i].begin(); pos != dag->OJPG[i].end(); pos++)
			(*pos)->is_deadline_updated = 0;
	}

	// update deadline from back
	// set initial jobs on all job lists
	vector<list<Node*>::reverse_iterator> cur_node;
	for(unsigned int i = 0; i < task_num; i++)
	{
		list<Node*>::reverse_iterator pos = dag->OJPG[i].rbegin();
		cur_node.push_back(pos);
	}
	unsigned int completed_task = 0;
	int *completed_task_index = (int*)malloc(sizeof(int)*task_num);
	for(unsigned int i = 0; i < task_num; i++)
		completed_task_index[i] = 0;
	while(completed_task < task_num)
	{
		for(unsigned int i = 0; i < task_num; i++)
		{
			if(completed_task_index[i] == 1)
				continue;
			if(all_successor_updated(*cur_node[i]))
			{
				update_deadline_for_each_node(*cur_node[i]);
				cur_node[i]++;		// go to the previous node
				if(cur_node[i] == dag->OJPG[i].rend())
				{
					completed_task_index[i] = 1;
					completed_task++;
				}
			}
		}
	}
	free(completed_task_index);
}

// update deadlines for jobs in min_finish_time_updated
void Execution::update_deadlines()
{
//	if(min_finish_time_updated.size() == 0)		// whole update
	{
		int updated_count = 1;
		while(updated_count > 0)
		{
			updated_count = 0;
			for(unsigned int i = 0; i < dag->OJPG.size(); i++)
			{
				list<Node*>::reverse_iterator pos;
				for(pos = dag->OJPG[i].rbegin(); pos != dag->OJPG[i].rend(); pos++)
				{
					int is_updated = update_deadline_for_each_node(*pos);
					updated_count += is_updated;
				}
			}
		}
	}
}

// pick the first node who is released and whose deadline is minimum
Node* Execution::get_the_first_node()
{
	Node *cur_node = NULL;
	int min_deadline = MAX_INT;
	list<Node*>::iterator pos;
	for(pos = ready_queue.begin(); pos != ready_queue.end(); pos++)
	{
		if((*pos)->release_time_PC <= cur_time)
		{
			if((*pos)->effective_deadline < min_deadline)
			{
				cur_node = (*pos);
				min_deadline = cur_node->effective_deadline;
			}
		}
	}
	return cur_node;
}

// get the nearest start time on PC after cur_time
int Execution::get_nearest_start_time()
{
	int min_start = MAX_INT;
	if(ready_queue.empty())
		return min_start;

	list<Node*>::iterator pos;
	for(pos = ready_queue.begin(); pos != ready_queue.end(); pos++)
	{
		if((*pos)->release_time_PC > cur_time)
		{
			if((*pos)->release_time_PC < min_start)
				min_start = (*pos)->release_time_PC;
		}
	}

	return min_start;
}

// find the task index who is running on the same ECU
void Execution::find_index_same_hw_id(int hw_id, int *start, int *end)
{
	for(unsigned int i = 0; i < dag->task_list.size(); i++)
	{
		if(dag->task_list[i]->hw_id == hw_id)
		{
			*start = i;
			break;
		}
	}
	for(unsigned int i = dag->task_list.size()-1; i >= 0; i--)
	{
		if(dag->task_list[i]->hw_id == hw_id)
		{
			*end = i;
			break;
		}
	}
}

// best_worst 0: best, 1: worst
// always based on priority preset in the offline phase
void Execution::emulate_schedule(int best_worst, Node *target)
{
	list<Node*> target_jobs;
	list<Node*>::iterator pos;
	for(int i = 0; i < target->F_set.size(); i++)
		target_jobs.push_back(target->F_set[i]);

	int cur_time = 0;

	// set remaining execution time based on best_worst
	for(pos = target_jobs.begin(); pos != target_jobs.end(); pos++)
	{
		if((*pos)->is_executed == 1)
			(*pos)->remaining_time_ECU = (*pos)->actual_execution_time_ECU;
		else if(best_worst == 0)
			(*pos)->remaining_time_ECU = (*pos)->task->bcet_ECU;
		else
			(*pos)->remaining_time_ECU = (*pos)->task->wcet_ECU;
	}

	Node* running_job = NULL;
	int min_release = MAX_INT;
	int min_priority = MAX_INT;
	Node* candidate;

	while(!target_jobs.empty())
	{
		// get min priority job
		min_priority = MAX_INT;
		candidate = NULL;
		for(pos = target_jobs.begin(); pos != target_jobs.end(); pos++)
		{
			if((*pos)->release_time <= cur_time && (*pos)->priority < min_priority)
			{
				min_priority = (*pos)->priority;
				candidate = (*pos);
			}
		}

		// get the next nearest release time
		min_release = MAX_INT;
		for(pos = target_jobs.begin(); pos != target_jobs.end(); pos++)
		{
			if((*pos)->release_time > cur_time && (*pos)->release_time < min_release)
				min_release = (*pos)->release_time;
		}
		if(candidate == NULL)
		{
			cur_time = min_release;
			continue;
		}

		int is_updated = 0;
		// start time check
		if(best_worst == 0 && candidate->remaining_time_ECU == candidate->task->bcet_ECU)
		{
			if(candidate->min_start_time_ECU != cur_time)
			{
				is_updated = 1;
				candidate->min_start_time_ECU = cur_time;
			}
		}
		else if(best_worst == 1 && candidate->remaining_time_ECU == candidate->task->wcet_ECU)
		{
			if(candidate->max_start_time_ECU != cur_time)
			{
				is_updated = 1;
				candidate->max_start_time_ECU = cur_time;
			}
			if(candidate->task->is_read == 1)
				candidate->release_time_PC = candidate->max_start_time_ECU;
		}

		// this job can finish
		if(candidate->remaining_time_ECU <= (min_release-cur_time))
		{
			cur_time += candidate->remaining_time_ECU;
			if(best_worst == 0)
			{
				if(candidate->min_finish_time_ECU != cur_time)
				{
					is_updated = 1;
					candidate->min_finish_time_ECU = cur_time;
				}
			}
			else if(best_worst == 1)
			{
				if(candidate->max_finish_time_ECU != cur_time)
				{
					is_updated = 1;
					candidate->max_finish_time_ECU = cur_time;
				}
			}

			// update finish time for virtual node
			if(candidate->task->is_write == 1)
			{
				Node *v = dag->find_virtual_node(candidate); // find virtual node
				if(v != NULL && best_worst == 0)
				{
					v->min_finish_time_ECU = cur_time;
					dag->push_job_for_update_deadline(v);
					dag->push_job_for_update_link(v);
				}
				else if(v != NULL && best_worst == 1)
					v->max_finish_time_ECU = cur_time;
			}
			candidate->remaining_time_ECU = 0;
			target_jobs.remove(candidate);
		}
		else	// just go to the next release
		{
			candidate->remaining_time_ECU -= (min_release-cur_time);
			cur_time = min_release;
		}

		// ready for resolve non-determinism
		if(is_updated == 1)
			dag->push_job_for_update_link(candidate);
	}
}

void Execution::update_start_finish_time(Node* executed)
{
	// initialize link updatable nodes
	dag->link_updatable.clear();

	// find task index
	int start_index, end_index;
	find_index_same_hw_id(executed->task->hw_id, &start_index, &end_index);

	// find a job who is maximally affected by executed.
	// for this, we use F-set
	Node *target = dag->max_jobs_in_F_set(executed, start_index, end_index);

	// reschedule based on BCET, WCET
	emulate_schedule(0, target);
	emulate_schedule(1, target);
}

// adjust non-deterministic predecessors, successors into deterministic or removed
void Execution::adjust_non_deterministic(Node* target)
{
	if(target->non_deterministic_predecessors.empty())
		return;
	list<Node*>::iterator pos = target->non_deterministic_predecessors.begin();
	while(pos != target->non_deterministic_predecessors.end())
	{
		if(target->is_virtual == 0)
		{
			if((*pos)->max_start_time_ECU < target->min_start_time_ECU)	// deterministic
			{
				Node *t = (*pos);
				pos++;
				dag->convert_determinitic(t, target);
				dag->push_job_for_update_deadline(t);
			}
			else if((*pos)->min_start_time_ECU >= target->max_start_time_ECU)	// removed
			{
				Node *t = (*pos);
				pos++;
				dag->unset_precedence_relation(0, t, target);
			}
			else
				pos++;
		}
		else
		{
			if((*pos)->max_start_time_ECU < target->min_finish_time_ECU)	// deterministic
			{
				Node *t = (*pos);
				pos++;
				dag->convert_determinitic(t, target);
				dag->push_job_for_update_deadline(t);
			}
			else if((*pos)->min_start_time_ECU >= target->max_finish_time_ECU)	// removed
			{
				Node *t = (*pos);
				pos++;
				dag->unset_precedence_relation(0, t, target);
			}
			else
				pos++;
		}
	}

	if(target->non_deterministic_successors.empty())
		return;
	pos = target->non_deterministic_successors.begin();
	while(pos != target->non_deterministic_successors.end())
	{
		if((*pos)->is_virtual == 0)
		{
			if(target->max_start_time_ECU < (*pos)->min_start_time_ECU)	// deterministic
			{
				Node *t = (*pos);
				pos++;
				dag->convert_determinitic(target, t);
				dag->push_job_for_update_deadline(target);
			}
			else if(target->min_start_time_ECU >= (*pos)->max_start_time_ECU)	// removed
			{
				Node *t = (*pos);
				pos++;
				dag->unset_precedence_relation(0, target, t);
			}
			else
				pos++;
		}
		else
		{
			if(target->max_start_time_ECU < (*pos)->min_finish_time_ECU)	// deterministic
			{
				Node *t = (*pos);
				pos++;
				dag->convert_determinitic(target, t);
				dag->push_job_for_update_deadline(target);
			}
			else if(target->min_start_time_ECU >= (*pos)->max_finish_time_ECU)	// removed
			{
				Node *t = (*pos);
				pos++;
				dag->unset_precedence_relation(0, target, t);
			}
			else
				pos++;
		}
	}
}

#ifdef LINUX_MODE
void Execution::update_time(int index)
{
	elapsed_time[index] += (end_time_measure.tv_sec-start_time_measure.tv_sec)*1000000 + 
		(end_time_measure.tv_usec-start_time_measure.tv_usec);
	times[index]++;
}
#endif

// execute jobs on PC
int Execution::execute_nodes_on_PC()
{
	// calculate number of jobs which will be executed until end_time
	vector<int> num_jobs;
	for(unsigned int i = 0; i < dag->task_list.size(); i++)
		num_jobs.push_back(end_time/dag->task_list[i]->period);
	int num_tasks = dag->task_list.size();

	update_deadlines();

	// set initial jobs in the ready queue
	for(int i = 0; i < num_tasks; i++)
	{
		if(dag->OJPG[i].front()->predecessors.empty())
			ready_queue.push_back(dag->OJPG[i].front());
	}

	// do schedule
	while(!ready_queue.empty())
	{
		int next_release_of_executed_job = 0;

		// pick the first node
		// (1) pick a job in the simulation ready queue
		Node *cur_node = get_the_first_node();
		if(cur_node == NULL)	// no job. then jump
		{
			int min_release = get_nearest_start_time();			// get the nearest start time
			cur_time = min_release;			// set cur_time to min_release
			continue;
		}

		// first start of this job
		if(cur_node->remaining_time_PC == cur_node->actual_execution_time_PC)		// start time of this node
			cur_node->start_time_PC = cur_time;

		// get start time of a job who can delay cur_node
		int next_time = get_nearest_start_time();

		if(cur_node->remaining_time_PC <= (next_time-cur_time))		// this node can complete
		{
			cur_time += cur_node->remaining_time_PC;	// cur_time increases
			cur_node->remaining_time_PC = 0;
			cur_node->actual_execution_time_ECU = cur_node->actual_execution_time_PC*100/cur_node->task->modified_rate;
			cur_node->is_executed = 1;
			cur_node->finish_time_PC = cur_time;		// finish time update
			next_release_of_executed_job = cur_node->release_time + dag->hyper_period;

			// simulatability check
			if(cur_node->is_virtual == 1 && cur_node->finish_time_PC > cur_node->min_finish_time_ECU)
				return 0;	// not simulatable

			// pop this job on the ready queue
			ready_queue.remove(cur_node);

			// clear jobs whose deadline would be changed
			dag->deadline_updatable.clear();

			// gather who can be in ready queue after cur_node is executed
			list<Node*> ready_candidates;
			list<Node*>::iterator pos;
			for(pos = cur_node->successors.begin(); pos != cur_node->successors.end(); pos++)
				ready_candidates.push_back(*pos);
			for(pos = cur_node->non_deterministic_successors.begin(); pos != cur_node->non_deterministic_successors.end(); pos++)
				ready_candidates.push_back(*pos);

			// kswe. (2) add a now job into OJPG
			dag->pop_and_push_node(cur_node);
			
			// kswe. (3) update start, finish times
			update_start_finish_time(cur_node);

			// kswe. (4) adjust non-determinism
			for(pos = dag->link_updatable.begin(); pos != dag->link_updatable.end(); pos++)
				adjust_non_deterministic(*pos);

			// kswe. (5) update effective deadlines
			update_deadlines_optimized();

			// update ready queue
			for(pos = ready_candidates.begin(); pos != ready_candidates.end(); pos++)
			{
				if((*pos)->predecessors.empty())
					ready_queue.push_back(*pos);
			}
		}
		else
		{
			cur_node->remaining_time_PC -= (next_time-cur_time);	// remaining time decreases
			cur_time += (next_time-cur_time);					// cur_time increases
		}
	}

	return 1;		// this system is simulatable
}
