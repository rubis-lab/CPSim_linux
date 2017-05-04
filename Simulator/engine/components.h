#ifndef __COMPONENTCLASSH_
#define __COMPONENTCLASSH_

#define DEBUG_MODE		0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <math.h>

#define RAND_RANGE(a,b) (((float)rand()/(float)RAND_MAX)*((float)b-(float)a)+(float)a)
#define MAX_INT 0x7fffffff

#define TASK(X) {Task##X}

#define MAX_RESOURCES		100
#define MAX_TASKS			1000
#define MAX_TASKS_ON_ECU	100

#define MAX_PRIORITY		1000

using namespace std;

class Component;
class Resource;
class Task_info;
class Job;
class Node;
class Scheduler_Universal;
class DAG;

class Component
{
public:
	int id;

	Component();
	Component(int);
	~Component();
};

class Resource : public Component
{
public:
	int type;		// 1: processor, 2: network
	int ratio;		// performance ratio. maximum: 100, minimum: 1
	int speed;		// clock (MHz) or baud rate (kbps)
	char resource_name[20];

	Resource();
	Resource(int, int, int, int, char*);
	~Resource();

	Scheduler_Universal *scheduler_link;

	int get_type();
	int get_ratio();
	int get_speed();
	char* get_resource_name();
};

class Task_info : public Component
{
public:
	int type;				// 0: computation, 1: communication
	int hw_id;				// id of actual HW which executes this task
	int period;
	int bcet_ECU;				// bcet on the target processor (on the ECU)
	int wcet_ECU;				// wcet on the target processor (on the ECU)
	int modified_rate;		// relative ratio between the execution time on the ECU and the execution time on PC
	int bcet_PC;		// bcet on the host processor (on the simulator)
	int wcet_PC;		// wcet on the host processor (on the simulator)
	int phase;
	int visible;			// 0: invisible, 1: visible
	int is_read;
	int is_write;
	int r_deadline;
	int priority;
	int a_deadline;
	int next_release_time;
	int next_job_id;
	char task_name[20];

	Task_info();
	Task_info(int, int, int, int, int, int, int, int, int, int, int, int, int, char*);
	~Task_info();

	// data dependency
	vector<Task_info*> successors;
	vector<Task_info*> predecessors;
};

/* Task function class
 */
class Task : public Component
{
public:
	Task();
	~Task();

	float data[10];
	int data_size;

	virtual void read() = 0;
	virtual void procedure() = 0;
	virtual void write() = 0;
};

// event-driven scheduler
class Scheduler_Universal
{
public:
	unsigned int num_tasks;
	int policy;				// 0: RM, 1: DM, 2: EDF
	int is_preemptible;		// 0: not preemptible, 1: preemptible
	int hyper;				// hyperperiod of tasks
	vector<Task_info*> tasks;
	vector<list<Node*> > node_list;

	Scheduler_Universal();
	Scheduler_Universal(int policy_input, int is_preemptible_input);
	Scheduler_Universal(int policy_input, int is_preemptible_input, vector<Task_info*> tasks_input, vector<list<Node*> > node_list_input);
	~Scheduler_Universal();

	void add_task(Task_info *t);
	void generate_nodes(int start_time, int end_time);
	void set_priority(int policy);
	void generate_schedule(int best_worst, int is_preemptible = 1);
	int get_WCBP_start(Node *target);
	void get_S_set(Node *target);
	void get_F_set(Node *target);
	void get_sets_for_all_nodes();
	void do_schedule_initial(int start_time, int end_time);
};

class Node
{
public:
	int is_virtual;
	int job_id;
	int release_time;
	int release_time_PC;
	int deadline_ECU;
	int remaining_time_ECU;
	int remaining_time_PC;
	int priority;
	int start_time_ECU;
	int finish_time_ECU;
	int actual_execution_time_ECU;
	int actual_execution_time_PC;
	int min_start_time_ECU;
	int min_finish_time_ECU;
	int max_start_time_ECU;
	int max_finish_time_ECU;
	int start_time_PC;
	int finish_time_PC;
	int effective_deadline;
	int is_executed;
	int is_all_precedence_set;			// is this newly added in OJPG?
	int is_deadline_updated;

	Task_info *task;

	list<Node*> predecessors;	// deterministic predecessors
	list<Node*> successors;		// deterministic successors
	list<Node*> non_deterministic_predecessors;		// non deterministic predecessors
	list<Node*> non_deterministic_successors;		// non deterministic successors
	vector<Node*> S_set;
	vector<Node*> F_set;
	vector<Node*> P_set;

	Node(Task_info *t, int j_id, int r_time, int d);
	Node(int v, Node *target_node);	// virtual value
	Node(Node *target_node);		// copy node
	Node(Node *target_node, int j_id);	// make node with suitable job id
	~Node();
};

class DAG
{
protected:
	int check_is_successor(int is_deterministic, Node *pre, Node *succ);	// check whether succ is successor of pre or not

	// is_deterministic 1: deterministic relation
	// return 1: newly set, 0: already set
	int set_precedence_relation(int is_deterministic, Node *pre, Node *succ);
	void case1(Node *target);	// physical-read constraint
	void case2(Node *target);	// physical-write constraint
	void case3(int send_index, Node *target);	// producer-consumer constraint

	// find a node in target_dag with task_id, job_id.
	// target_dag 0: offline guider, 1: OJPG
	// if there isn't a node, then NULL is returned.
	Node* find_node(int target_dag, int task_id, int job_id);

	int get_ref_job_id(int task_id, int job_id);

public:
	DAG(vector<Task_info*> t, vector<Resource*> r, int h);
	~DAG();

	int hyper_period;
	vector<int> number_of_jobs_in_hyperperiod;
	vector<Task_info*> task_list;
	vector<list<Node*> > node_list;			// offline guider
	vector<list<Node*> > OJPG;
	list<Node*> deadline_updatable;
	list<Node*> link_updatable;

	// find a terminal node of target
	// if there isn't a node, then NULL is returned.
	Node* find_virtual_node(Node *target);
	void convert_determinitic(Node *pre, Node *succ);
	void unset_precedence_relation(int is_deterministic, Node *pre, Node *succ);
	void generate_offline_guider();	// generate the offline guider
	void generate_initial_OJPG();
	void unlink_node_in_OJPG(Node *target);	// unlink target and target's successors
	int is_node_in_OJPG(int task_id, int job_id);
	Node* max_jobs_in_F_set(Node *target, int start_index, int end_index);

	// add a new node to OJPG with task_id, job_id
	// multiple jobs may be added if job_id is big
	// return a node who has task_id, job_id
	Node* add_new_node_into_OJPG(int task_id, int job_id);

	// set default precedence relations recursively
	void set_default_precedence_relations(Node *target);

	void push_job_for_update_deadline(Node *target);
	void push_job_for_update_link(Node *target);

	// remove executed job and add a new job after hyper period
	void pop_and_push_node(Node *executed, int is_push = 1);
};

class Execution
{
public:
	Execution(int e_time, DAG *d, vector<Resource*> r);
	~Execution();

	DAG* dag;
	vector<Resource*> resources;
	list<Node *> ready_queue;

	unsigned long long cur_time;
	int end_time;

	// update effective deadline for target
	// return 0: not updated, 1: updated
	int update_deadline_for_each_node(Node *target);
	int all_successor_updated(Node *target);
	void update_deadlines_optimized();
	void update_deadlines_revised();
	void update_deadlines();
	int get_nearest_start_time();
	int check_all_predecessors_executed(Node *target);
	void find_index_same_hw_id(int hw_id, int *start, int *end);
	void emulate_schedule(int best_worst, Node *target);
	void update_start_finish_time(Node *executed);
	void adjust_non_deterministic(Node *target);
	Node* get_the_first_node();

	// execute nodes on PC
	// return 1: simulatable, 0: not simulatable
	int execute_nodes_on_PC();
};


#endif
