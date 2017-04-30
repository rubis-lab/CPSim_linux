#include "components.h"

DAG::DAG(vector<Task_info*> t, vector<Resource*> r, int h)
{
	task_list = t;
	hyper_period = h;

	// make full list for all nodes
	for(unsigned int i = 0; i < r.size(); i++)
	{
		for(int j = 0; j < r[i]->scheduler_link->node_list.size(); j++)
			node_list.push_back(r[i]->scheduler_link->node_list[j]);
	}

	// initialize nubmer of jobs in hyperperiod, OJPG list
	for(unsigned int i = 0; i < node_list.size(); i++)
	{
		int n = hyper_period/task_list[i]->period;
		number_of_jobs_in_hyperperiod.push_back(n);
		list<Node*> l;
		OJPG.push_back(l);
	}
}

// check whether succ is successor of pre or not
int DAG::check_is_successor(int is_deterministic, Node *pre, Node *succ)
{
	list<Node*>::iterator pos, end;
	if(is_deterministic == 1)
	{
		pos = pre->successors.begin();
		end = pre->successors.end();
	}
	else
	{
		pos = pre->non_deterministic_successors.begin();
		end = pre->non_deterministic_successors.end();
	}
	for(; pos != end; pos++)
	{
		if((*pos) == succ)
			return 1;
	}

	return 0;
}

// is_deterministic 1: deterministic relation
// return 1: newly set, 0: already set
int DAG::set_precedence_relation(int is_deterministic, Node *pre, Node *succ)
{
	int re = 0;
	if(is_deterministic == 1 && check_is_successor(is_deterministic, pre, succ) == 0)
	{
		pre->successors.push_back(succ);
		succ->predecessors.push_back(pre);
		re = 1;
	}
	else if(check_is_successor(1, pre, succ) == 0 && check_is_successor(is_deterministic, pre, succ) == 0)
	{
		pre->non_deterministic_successors.push_back(succ);
		succ->non_deterministic_predecessors.push_back(pre);
		re = 1;
	}
	return 0;
}

void DAG::unset_precedence_relation(int is_deterministic, Node *pre, Node *succ)
{
	list<Node*>::iterator pos;
	if(is_deterministic == 1)
	{
		// remove succ's predecessor
		succ->predecessors.remove(pre);

		// remove pre's successor
		pre->successors.remove(succ);
	}
	else
	{
		// remove succ's non-deterministic predecessor
		succ->non_deterministic_predecessors.remove(pre);

		// remove pre's non-deterministic successor
		pre->non_deterministic_successors.remove(succ);
	}
}

void DAG::convert_determinitic(Node *pre, Node *succ)
{
	unset_precedence_relation(0, pre, succ);
	set_precedence_relation(1, pre, succ);
}

// physical-read constraint
void DAG::case1(Node *target)
{
	if(target->min_start_time_ECU == target->max_start_time_ECU)
		return;

	// set links between jobs in S-set and the target
	for(unsigned int i = 0; i < target->S_set.size(); i++)
	{
		if(target->S_set[i]->max_start_time_ECU < target->min_start_time_ECU)			// set deterministic
			set_precedence_relation(1, target->S_set[i], target);
		else	// set non-deterministic
			set_precedence_relation(0, target->S_set[i], target);
	}
}

// physical-write constraint
void DAG::case2(Node *target)
{
	// insert a terminal node for finish time
//	Node *v = insert_virtual_node(3, target);
	Node *v = new Node(1, target);

	// set links between jobs in F-set and the target
	for(unsigned int i = 0; i < target->F_set.size(); i++)
	{
		if(target->F_set[i]->max_start_time_ECU < target->min_finish_time_ECU)			// set deterministic
			set_precedence_relation(1, target->F_set[i], v);
		else	// set non-deterministic
			set_precedence_relation(0, target->F_set[i], v);
	}
}

// producer-consumer constraint
void DAG::case3(int send_index, Node *target)
{
	Node *cur = NULL, *prev = NULL;
	vector<Node*> potential_producers;

	// find a job whose max(t^F,real) is less than target's min(t^S,real), and set link between the job and target
	list<Node*>::iterator pos;
	for(pos = node_list[send_index].begin(); pos != node_list[send_index].end(); pos++)
	{
		cur = (*pos);
		// if the job whose FT^W is greater than target's ST^B is the first job,
		// this job cannot be a sender of target
		if(prev != NULL)
		{
			// set link between this job and target
			set_precedence_relation(1, prev, target);
		}
		break;
		prev = cur;
	}

	// get potential producers
	for(pos = node_list[send_index].begin(); pos != node_list[send_index].end(); pos++)
	{
		if((*pos)->max_finish_time_ECU > target->min_start_time_ECU &&
			(*pos)->min_finish_time_ECU <= target->max_start_time_ECU)	// condition
			potential_producers.push_back(*pos);
	}

	if(potential_producers.empty())
		return;

	// set precedence relations for S-set of the target
	case1(target);

	// set precedence relations for F-set of potential producers
	// set links between jobs in F-set and the target
	for(unsigned int i = 0; i < potential_producers.size(); i++)
	{
		Node *producer = potential_producers[i];
		for(int j = 0; j < producer->F_set.size(); j++)
		{
			if(producer->F_set[j]->max_start_time_ECU < target->min_start_time_ECU)			// set deterministic
				set_precedence_relation(1, producer->F_set[j], target);
			else	// set non-deterministic
				set_precedence_relation(0, producer->F_set[j], target);
		}
	}
}

// find a node in target_dag with task_id, job_id.
// target_dag 0: offline guider, 1: OJPG
// if there isn't a node, then NULL is returned.
Node* DAG::find_node(int target_dag, int task_id, int job_id)
{
	list<Node*>::iterator pos;
	if(target_dag == 0)	// offline guider
	{
		for(pos = node_list[task_id].begin(); pos != node_list[task_id].end(); pos++)
		{
			if((*pos)->job_id == job_id)
				return (*pos);
		}
	}
	else if(target_dag == 1)	// OJPG
	{
		for(pos = OJPG[task_id].begin(); pos != OJPG[task_id].end(); pos++)
		{
			if((*pos)->job_id == job_id)
				return (*pos);
		}
	}
	return NULL;
}

// calculate reference job id in the offline guider based on job_id
int DAG::get_ref_job_id(int task_id, int job_id)
{
	return (job_id%number_of_jobs_in_hyperperiod[task_id])+number_of_jobs_in_hyperperiod[task_id];
}

// find a terminal node of target
// if there isn't a node, then NULL is returned.
Node* DAG::find_virtual_node(Node* target)
{
	list<Node*>::iterator pos;
	// find the virtual node in successors
	for(pos = target->successors.begin(); pos != target->successors.end(); pos++)
	{
		if((*pos)->is_virtual > 0 && (*pos)->job_id == target->job_id &&
			(*pos)->task->id == target->task->id)
			return (*pos);
	}

	// find it in non-deterministic successors
	for(pos = target->non_deterministic_successors.begin(); pos != target->non_deterministic_successors.end(); pos++)
	{
		if((*pos)->is_virtual > 0 && (*pos)->job_id == target->job_id &&
			(*pos)->task->id == target->task->id)
			return (*pos);
	}
	
	return NULL;
}

// generate the offline guider
void DAG::generate_offline_guider()
{
	for(unsigned int i = 0; i < node_list.size(); i++)
	{
		list<Node*>::iterator pos;
		Node *pre = NULL, *cur = NULL;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
		{
			cur = (*pos);
			if(pre != NULL)		// stateful
				set_precedence_relation(1, pre, cur);

			// manage physical-read constraints
			if(cur->task->is_read == 1)
				case1(cur);

			// manage physical-write constraints
			if(cur->task->is_write == 1)
				case2(cur);

			// manage producer-consumer constraints
			for(int j = 0; j < cur->task->predecessors.size(); j++)		// for all sender
			{
				int send_task_index = cur->task->predecessors[j]->id;
				case3(send_task_index, cur);
			}

			pre = (*pos);
		}
	}
}

void DAG::generate_initial_OJPG()
{
	// make nodes within the first hyperperiod
	for(unsigned int i = 0; i < node_list.size(); i++)
	{
		list<Node*>::iterator pos;
		for(pos = node_list[i].begin(); pos != node_list[i].end(); pos++)
		{
			if((*pos)->job_id < number_of_jobs_in_hyperperiod[i])
			{
				Node *n = new Node(*pos);
				n->is_all_precedence_set = 1;	// will be set in this function
				OJPG[i].push_back(n);
			}
			else
				break;
		}
	}

	// set precedence relations
	for(unsigned int i = 0; i < OJPG.size(); i++)
	{
		list<Node*>::iterator pos;
		for(pos = OJPG[i].begin(); pos != OJPG[i].end(); pos++)
		{
			Node *cur_node = (*pos);
			Node *node_in_guider = find_node(0, i, cur_node->job_id);

			// set deterministic predecessors
			list<Node*>::iterator pos_pre;
			for(pos_pre = node_in_guider->predecessors.begin(); pos_pre != node_in_guider->predecessors.end(); pos_pre++)
			{
				Node *pre_in_OJPG = find_node(1, (*pos_pre)->task->id, (*pos_pre)->job_id);
				set_precedence_relation(1, pre_in_OJPG, cur_node);
			}

			// set non-deterministic predecessors
			for(pos_pre = node_in_guider->non_deterministic_predecessors.begin(); pos_pre != node_in_guider->non_deterministic_predecessors.end(); pos_pre++)
			{
				Node *pre_in_OJPG = find_node(1, (*pos_pre)->task->id, (*pos_pre)->job_id);
				set_precedence_relation(0, pre_in_OJPG, cur_node);
			}

			// set S_set
			for(int j = 0; j < node_in_guider->S_set.size(); j++)
			{
				Node *job_in_OJPG = find_node(1, node_in_guider->S_set[j]->task->id, node_in_guider->S_set[j]->job_id);
				cur_node->S_set.push_back(job_in_OJPG);
			}

			// set F_set
			for(int j = 0; j < node_in_guider->F_set.size(); j++)
			{
				Node *job_in_OJPG = find_node(1, node_in_guider->F_set[j]->task->id, node_in_guider->F_set[j]->job_id);
				cur_node->F_set.push_back(job_in_OJPG);
			}

			// set P_set
			for(int j = 0; j < node_in_guider->P_set.size(); j++)
			{
				Node *job_in_OJPG = find_node(1, node_in_guider->P_set[j]->task->id, node_in_guider->P_set[j]->job_id);
				cur_node->P_set.push_back(job_in_OJPG);
			}
		}
	}
}

// unlink target and target's successors in OJPG
void DAG::unlink_node_in_OJPG(Node *target)
{
	// remove successors and corresponding predecessors
	while(!target->successors.empty())
	{
		Node *succ = target->successors.front();
		unset_precedence_relation(1, target, succ);
	}

	// remove non-deterministic ones
	while(!target->non_deterministic_successors.empty())
	{
		Node *succ = target->non_deterministic_successors.front();
		unset_precedence_relation(0, target, succ);
	}
	while(!target->non_deterministic_predecessors.empty())
	{
		Node *pre = target->non_deterministic_predecessors.front();
		unset_precedence_relation(0, pre, target);
	}
}

// return 1: found
int DAG::is_node_in_OJPG(int task_id, int job_id)
{
	if(OJPG[task_id].empty())
		return 0;
	int min_job_id = OJPG[task_id].front()->job_id;
	int max_job_id = OJPG[task_id].back()->job_id;
	if(min_job_id <= job_id && job_id <= max_job_id)
		return 1;
	else
		return 0;
}

// return a node whose F-set has a and the size is maximum
Node* DAG::max_jobs_in_F_set(Node *target, int start_index, int end_index)
{
	Node *re = target;
	int num = target->F_set.size();
	list<Node*>::iterator pos;
	for(int i = start_index; i <= end_index; i++)
	{
		for(pos = OJPG[i].begin(); pos != OJPG[i].end(); pos++)
		{
			int is_in_fset = 0;
			for(int j = 0; j < (*pos)->F_set.size(); j++)
			{
				if((*pos)->F_set[j] == target)
				{
					is_in_fset = 1;
					if((*pos)->F_set.size() > num)
					{
						re = (*pos);
						num = re->F_set.size();						
					}
				}
			}
			if(is_in_fset == 0)		// remaining jobs do not have target in F-set
				break;
		}
	}
	return re;
}

// add a new node into OJPG with task_id, job_id
// multiple jobs may be added if job_id is big
// return a node who has task_id, job_id
Node* DAG::add_new_node_into_OJPG(int task_id, int job_id)
{
	Node *re = NULL;

	// already exists, do nothing
	if(is_node_in_OJPG(task_id, job_id) == 1)
		return find_node(1, task_id, job_id);

	// get the last job id of this task in OJPG
	int max_job_id = OJPG[task_id].back()->job_id;

	for(unsigned int i = max_job_id+1; i <= job_id; i++)
	{
		// get a reference node within the second hyperperiod in the offline guider
		Node *ref_node = find_node(0, task_id, get_ref_job_id(task_id, job_id));

		// make a new node for OJPG and add to OJPG
		Node *n = new Node(ref_node, i);

		// set actual execution time
		if(n->is_virtual == 0)
		{
			n->actual_execution_time_ECU = rand()%(n->task->wcet_ECU-n->task->bcet_ECU+1) + n->task->bcet_ECU;
			n->remaining_time_ECU = n->actual_execution_time_ECU;
			n->actual_execution_time_PC = n->actual_execution_time_ECU*n->task->modified_rate/100;
			n->remaining_time_PC = n->actual_execution_time_PC;
		}

		OJPG[task_id].push_back(n);
		re = n;
	}
	return re;
}

void DAG::set_default_precedence_relations(Node *target)
{
	if(target->is_all_precedence_set == 1)
		return;

	int task_id = target->task->id;
	int ref_job_id = get_ref_job_id(task_id, target->job_id);
	int num_hyperperiod = (target->job_id - ref_job_id)/number_of_jobs_in_hyperperiod[task_id];

	// get ref_node in the offline guider
	Node *ref_node = find_node(0, task_id, ref_job_id);
	list<Node*>::iterator pos;

	// add and set deterministic predecessors into OJPG
	for(pos = ref_node->predecessors.begin(); pos != ref_node->predecessors.end(); pos++)
	{
		int job_id = (*pos)->job_id + (num_hyperperiod*number_of_jobs_in_hyperperiod[(*pos)->task->id]);
		if(OJPG[(*pos)->task->id].front()->job_id > job_id)
			continue;
		Node *pre = find_node(1, (*pos)->task->id, job_id);
		if(pre == NULL)		// add this node into OJPG
		{
			add_new_node_into_OJPG((*pos)->task->id, job_id);
			pre = find_node(1, (*pos)->task->id, job_id);
		}
		set_precedence_relation(1, pre, target);
		set_default_precedence_relations(pre);
	}

	// add and set non-deterministic predecessors into OJPG
	for(pos = ref_node->non_deterministic_predecessors.begin(); pos != ref_node->non_deterministic_predecessors.end(); pos++)
	{
		int job_id = (*pos)->job_id + (num_hyperperiod*number_of_jobs_in_hyperperiod[(*pos)->task->id]);
		if(OJPG[(*pos)->task->id].front()->job_id > job_id)
			continue;
		Node *pre = find_node(1, (*pos)->task->id, job_id);
		if(pre == NULL)		// add this node into OJPG
		{
			add_new_node_into_OJPG((*pos)->task->id, job_id);
			pre = find_node(1, (*pos)->task->id, job_id);
		}
		set_precedence_relation(0, pre, target);
		set_default_precedence_relations(pre);
	}

	target->is_all_precedence_set = 1;
}


void DAG::push_job_for_update_deadline(Node *target)
{
	if(deadline_updatable.empty())
	{
		deadline_updatable.push_back(target);
		return;
	}
	list<Node*>::iterator pos;
	for(pos = deadline_updatable.begin(); pos != deadline_updatable.end(); pos++)
	{
		// already exist
		if((*pos) == target)
			return;

		// check whether successors of target exist
		if(!target->successors.empty())
		{
			list<Node*>::iterator succ;
			for(succ = target->successors.begin(); succ != target->successors.end(); succ++)
			{
				if((*pos) == (*succ))
					return;
			}
		}
	}

	deadline_updatable.push_back(target);
}

void DAG::push_job_for_update_link(Node *target)
{
	if(link_updatable.empty())
	{
		link_updatable.push_back(target);
		return;
	}
	list<Node*>::iterator pos;
	for(pos = link_updatable.begin(); pos != link_updatable.end(); pos++)
	{
		// already exist
		if((*pos) == target)
			return;
	}

	deadline_updatable.push_back(target);
}

// remove executed job and add a new job after hyper period
void DAG::pop_and_push_node(Node *executed, int is_push)
{
	if(executed->is_virtual == 0)
	{
		if(executed != OJPG[executed->task->id].front())
			printf("bug\n");
		// pop this node
		OJPG[executed->task->id].pop_front();
	}

	// unlink
	unlink_node_in_OJPG(executed);

	if(is_push == 0)
		return;
	
	// add a new job into OJPG
	int job_id = executed->job_id + number_of_jobs_in_hyperperiod[executed->task->id];
	Node *new_node = add_new_node_into_OJPG(executed->task->id, job_id);

	// set default precedence relations
	set_default_precedence_relations(new_node);

	// add this job for updating deadline
	push_job_for_update_deadline(new_node);
}

