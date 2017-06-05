#ifndef __COMPONENTCLASSH_
#define __COMPONENTCLASSH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <math.h>

#define RAND_RANGE(a,b) (((float)rand()/(float)RAND_MAX)*((float)b-(float)a)+(float)a)
#define MAX_INT 0x7fffffff

#define TASK(X) {Task##X}

#define MAX_RESOURCES 100
#define MAX_TASKS 1000

#define PERIOD 0
#define DEADLINE 1

#define FIXED 0
#define DYNAMIC 1

#define NON_PREEMPTABLE 0
#define PREEMPTABLE 1

enum {START, COMPLETE, RESUMED};
enum {RM, DM, EDF, CPS, CUSTOM};	// for scheduling policy

using namespace std;

class Component;
class Scheduler;
class Resource;
class Task_info;
class Event;
class Task;

/* Component class
 * This is a super class of Resource, Task_info class
 */
class Component
{
protected:
	int id;

public:
	Component();
	Component(int);
	~Component();
	int get_id();
	void set_id(int);
};

/* for schedule plotting */
class Time_plot
{
private:
	int is_start;				// 1: start, 0: end, some value for response time
	unsigned long long time;	// time
	int task_num;				// task number
	char task_name[20];			// task name
	char resource_name[20];		// resource name

public:
	Time_plot();
	Time_plot(int, unsigned long long, int);
	Time_plot(int, unsigned long long, int, char*, char*);
	~Time_plot();

	int get_is_start();
	unsigned long long get_time();
	int get_task_num();
	char* get_task_name();
	char* get_resource_name();
};

/* Scheduler class
 * This class is for event-driven scheduler
 */
class Scheduler
{
private:
	int p_policy;			// priority assign policy. 0: fixed, 1: dynamic
	int p_standard;			// standard for priority setting. 0: period, 1: deadline
	int preemptable;		// preemption in execution. 1: preemptable, 0: non-preemptable
	int componentizing;		// componentizing (optional). 0: not componentized, 1: componentized
	int current_time;		// time that schedule have been created until now

protected:
    int get_task_index_from_task_id(int);
	void modify_time(list<Event*>*, int);
	void resort(list<Event*>*);
	void process_completion(list<Event*>*, Event*, int, list<Time_plot *>*);
	void add_time_plot(int, unsigned long long, Event*, list<Time_plot *>*);

public:
	Scheduler();
	Scheduler(int, int, Resource*);
	~Scheduler();

	void insert_event(list<Event*>*, Event*);

	int get_p_policy();
	int get_p_standard();
	int get_preemptable();
	int get_componentizing();
	int get_current_time();
	void set_current_time(int);
	int extract_schedule(int, int, list<Time_plot *>*);
	void set_priority();

	Resource *resource_link;
	vector<Task_info*> tasks;

	list<Event*> log;			// event log for over time window
	list<Event*> log2;			// event log for current time window (use for setting release time)
	list<Event*> log3;			// event log who has start events only (use for setting deadline)
};

/* Event class
 * This class denotes an event in the scheduler
 * An event consists of three kinds of behaviors, i.e., start, completion, resume.
 */
class Event
{
private:
	int task_id;	// task id
	int job_id;		// job id who create this event
	int type;		// event type. 0: start, 1: completion, 2: resumed
	int time;		// time when this event occur
	int remaining_time;	// remaining execution time of this job

public:
	Event();
	Event(int, int, int, Task_info*, Task*);
	~Event();

	Task_info *task_info_link;
	Task *task_link;

	int get_task_id();
	int get_job_id();
	int get_type();
	int get_time();
	int get_remaining_time();
	void set_task_id(int);
	void set_job_id(int);
	void set_type(int);
	void set_time(int);
	void set_remaining_time(int);
};

/* Resource class
 * This class is a super class of all resources such as ECU, CAN
 * Each resource has its own scheduler.
 */
class Resource : public Component
{
private:
	int type;		// 1: processor, 2: network
	int ratio;		// performance ratio. maximum: 100, minimum: 1
	int speed;		// clock (MHz) or baud rate (kbps)
	char resource_name[20];

public:
	Resource();
	Resource(int, int, int, int, char*);
	~Resource();

	Scheduler *scheduler_link;

	int get_type();
	int get_ratio();
	int get_speed();
	char* get_resource_name();
};

/* Task Info class
 * This class describes task's static properties such as period, worst-case execution time, etc.
 * Also this class has links to its own jobs
 */
class Task_info : public Component
{
private:
	int type;				// 0: computation, 1: communication
	int period;
	int wcet;				// wcet on the target processor (on the ECU)
	int modified_wcet;		// wcet on the host processor (on the simulator)
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

public:
	Task_info();
	Task_info(int, int, int, int, int, int, int, int, int, int, int, char*, Scheduler*);
	~Task_info();

	Scheduler *scheduler_link;

	// data dependency
	vector<Task_info*> successors;		// task level successor
	vector<Task_info*> predecessors;	// task level predecessor

	list<Task*> task_instances;		// links to jobs

	int get_type();
	int get_period();
	int get_wcet();
	int get_modified_wcet();
	int get_phase();
	int get_visible();
	int get_is_read();
	int get_is_write();
	int get_r_deadline();
	int get_priority();
	int get_a_deadline();
	int get_next_release_time();
	int get_next_job_id();
	char* get_task_name();
	void set_visible(int);
	void set_priority(int);
	void set_a_deadline(int);
	void set_next_release_time(int);
	void job_id_increase();		// increase next job id
	Task* find_task(int);
};


/* Task class
 * This class describes task's dynamic properties such as release time, absolute deadline, etc.
 * A task's job is an instanse of this class
 */
class Task : public Component
{
protected:
	int job_id;
	int release_time;
	int a_deadline;
	int effective_deadline;
	int effective_release_time;
	int is_waiting;				// is waiting or not
	unsigned long long start_time;		// actual time when this job starts its execution
	unsigned long long completion_time;	// actual time when this job completes its execution

public:
	Task();
	Task(Task_info*);
	Task(int, int);
	Task(int, int, Task_info*);
	Task(int, int, int, int, Task_info*);
	~Task();

    // default internal variables for communicating with other tasks
    float internal_data[10];
	Task_info *task_link;

	// data dependency
	vector<Task*> predecessors;		// job level predecessor
	vector<Task*> successors;		// job level successor

	virtual void procedure() = 0;
	virtual void write() = 0;

	int get_job_id();
	int get_release_time();
	int get_a_deadline();
	int get_effective_deadline();
	int get_effective_release_time();
	int get_is_waiting();
	unsigned long long get_start_time();
	unsigned long long get_completion_time();

	void set_job_id(int);
	void set_release_time(int);
	void set_a_deadline(int);
	void set_effective_deadline(int);
	void set_effective_release_time(int);
	void set_waiting();
	void set_start_time(unsigned long long);
	void set_completion_time(unsigned long long);
};

/* Plan class
 * This class serializes all generated events in Scheduler classes for executing a single-core processor
 */
class Plan
{
private:
	void set_effective_release_time(vector<Resource*>*, vector<Task_info*>*);
	void set_effective_deadline(vector<Resource*>*);

public:
	Plan();
	~Plan();

	void generate_plan(vector<Resource*>*, vector<Task_info*>*);
};

/* Execution class
 * This class is responsible for actual executions of tasks in the simulator
 */
class Execution
{
public:
	Execution();
	~Execution();

	Task* get_next_job(int, vector<Task_info*>*);
};

#endif
