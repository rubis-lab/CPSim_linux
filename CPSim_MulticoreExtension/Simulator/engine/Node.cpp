#include "components.h"

Node::Node(Task_info *t, int j_id, int r_time, int d)
{
	is_virtual = 0;
	task = t;
	job_id = j_id;
	release_time = r_time;
	release_time_PC = 0;
	deadline_ECU = d;
	effective_deadline = MAX_INT-1;
	min_start_time_ECU = max_start_time_ECU = -1;
	min_finish_time_ECU = max_finish_time_ECU = -1;
	start_time_ECU = finish_time_ECU = -1;
	start_time_PC = finish_time_PC = -1;
	is_executed = -1;
	is_all_precedence_set = -1;
	is_deadline_updated = -1;
}

Node::Node(int v, Node *target_node)
{
	is_virtual = v;
	task = target_node->task;
	job_id = target_node->job_id;
	release_time = target_node->release_time;
	release_time_PC = 0;
	min_finish_time_ECU = target_node->min_finish_time_ECU;
	max_finish_time_ECU = target_node->max_finish_time_ECU;
	deadline_ECU = target_node->deadline_ECU;
	remaining_time_ECU = remaining_time_PC = actual_execution_time_ECU = actual_execution_time_PC = 0;
	effective_deadline = target_node->min_finish_time_ECU;
	is_executed = -1;
	is_all_precedence_set = -1;
	is_deadline_updated = -1;
}

Node::Node(Node *target_node)
{
	is_virtual = target_node->is_virtual;
	job_id = target_node->job_id;
	release_time = target_node->release_time;
	deadline_ECU = target_node->deadline_ECU;
	remaining_time_ECU = target_node->remaining_time_ECU;
	remaining_time_PC = target_node->remaining_time_PC;
	priority = target_node->priority;
	start_time_ECU = target_node->start_time_ECU;
	finish_time_ECU = target_node->finish_time_ECU;
	actual_execution_time_ECU = target_node->actual_execution_time_ECU;
	actual_execution_time_PC = target_node->actual_execution_time_PC;
	min_start_time_ECU = target_node->min_start_time_ECU;
	min_finish_time_ECU = target_node->min_finish_time_ECU;
	max_start_time_ECU = target_node->max_start_time_ECU;
	max_finish_time_ECU = target_node->max_finish_time_ECU;
	start_time_PC = target_node->start_time_PC;
	finish_time_PC = target_node->finish_time_PC;
	effective_deadline = target_node->effective_deadline;
	if(target_node->task->is_read == 1)
		release_time_PC = max_start_time_ECU;
	else
		release_time_PC = 0;

	task = target_node->task;
	is_all_precedence_set = -1;
	is_deadline_updated = -1;
	is_executed = -1;
}

Node::Node(Node *target_node, int j_id)
{
	task = target_node->task;
	int time_interval = (j_id-target_node->job_id)*task->period;

	is_virtual = target_node->is_virtual;
	job_id = j_id;
	release_time = target_node->release_time + time_interval;
	deadline_ECU = target_node->deadline_ECU + time_interval;
	priority = target_node->priority;
	start_time_ECU = target_node->start_time_ECU + time_interval;
	finish_time_ECU = target_node->finish_time_ECU + time_interval;
	actual_execution_time_ECU = target_node->actual_execution_time_ECU;
	remaining_time_PC = actual_execution_time_PC = target_node->actual_execution_time_PC;
	min_start_time_ECU = target_node->min_start_time_ECU + time_interval;
	min_finish_time_ECU = target_node->min_finish_time_ECU + time_interval;
	max_start_time_ECU = target_node->max_start_time_ECU + time_interval;
	max_finish_time_ECU = target_node->max_finish_time_ECU + time_interval;
	start_time_PC = target_node->start_time_PC;
	finish_time_PC = target_node->finish_time_PC;
	effective_deadline = target_node->effective_deadline;
	is_executed = -1;

	if(target_node->task->is_read == 1 && is_virtual == 0)
		release_time_PC = max_start_time_ECU;
	else
		release_time_PC = 0;

	is_all_precedence_set = -1;
	is_deadline_updated = -1;
}