#include "task_created.hh"

Time_plot::Time_plot()
{
}

// constructor for Time_plot class
Time_plot::Time_plot(int response_time, unsigned long long time_input, int num_input)
{
	is_start = response_time;
	time = time_input;
	task_num = num_input;
}

// constructor for Time_plot class
Time_plot::Time_plot(int start_input, unsigned long long time_input, int num_input, char *task_name_input, char *resource_name_input)
{
	is_start = start_input;
	time = time_input;
	task_num = num_input;
	strcpy(task_name, task_name_input);
	strcpy(resource_name, resource_name_input);
}

Time_plot::~Time_plot()
{
}

// This function returns 'is_start' variable in Time_plot class
int Time_plot::get_is_start()
{
	return is_start;
}

// This function returns 'time' variable in Time_plot class
unsigned long long Time_plot::get_time()
{
	return time;
}

// This function returns 'task_num' variable in Time_plot class
int Time_plot::get_task_num()
{
	return task_num;
}

// This function returns 'task_name' variable in Time_plot class
char* Time_plot::get_task_name()
{
	return task_name;
}

// This function returns 'resource_name' variable in Time_plot class
char* Time_plot::get_resource_name()
{
	return resource_name;
}

// default constructor for Scheduler class
Scheduler::Scheduler()
{
	current_time = 0;
	componentizing = 0;
	resource_link = NULL;
}

// constructor for Scheduler class
Scheduler::Scheduler(int policy_input, int preempt_input, Resource *resource)
{
	p_policy = 0;
	p_standard = 0;
	componentizing = 0;
	preemptable = preempt_input;
	if(policy_input == RM)
	{
		p_policy = 0;
		p_standard = 0;
	}
	else if(policy_input == DM)
	{
		p_policy = 0;
		p_standard = 1;
	}
	else if(policy_input == EDF)
	{
		p_policy = 1;
		p_standard = 1;
	}
	else if(policy_input == CPS)
	{
		p_policy = 1;
		p_standard = 1;
		componentizing = 1;
	}
	current_time = 0;
	resource_link = resource;
	resource->scheduler_link = this;
}

Scheduler::~Scheduler()
{
}

// This function returns 'p_policy' variable in Scheduler class
int Scheduler::get_p_policy()
{
	return p_policy;
}

// This function returns 'p_standard' variable in Scheduler class
int Scheduler::get_p_standard()
{
	return p_standard;
}

// This function returns 'preemptable' variable in Scheduler class
int Scheduler::get_preemptable()
{
	return preemptable;
}

// This function returns 'componentizing' variable in Scheduler class
int Scheduler::get_componentizing()
{
	return componentizing;
}

// This function returns 'current_time' variable in Scheduler class
int Scheduler::get_current_time()
{
	return current_time;
}

// This function sets 'current_time' variable in Scheduler class as 'input'
void Scheduler::set_current_time(int input)
{
	current_time = input;
}

/* This function inserts Event class into 'header' list.
 * Each class is inserted into the with time and priority order.
 */
void Scheduler::insert_event(list<Event*> *header, Event *target)
{
	// first element
	if(header->empty())
	{
		header->push_back(target);
		return;
	}

	list<Event*>::iterator pos;
	for(pos = header->begin(); pos != header->end(); pos++)
	{
		if(((*pos)->get_time() > target->get_time()) ||
			((*pos)->get_time() == target->get_time() &&
			(*pos)->task_info_link->get_priority() > target->task_info_link->get_priority()))
		{
			header->insert(pos, target);
			return;
		}
	}
	header->push_back(target);		// push target to the last position
}

// This function re-sorts 'header' list with modified priority
void Scheduler::resort(list<Event*>* header)
{
	list<Event*> temp_list;
	Event *temp_event;
	while(!header->empty())
	{
		temp_event = header->front();
		header->pop_front();
		insert_event(&temp_list, temp_event);
	}
	
	while(!temp_list.empty())
	{
		temp_event = temp_list.front();
		temp_list.pop_front();
		insert_event(header, temp_event);
	}
}

/* This function sets priorities of all tasks in the resource who has this scheduler.
 * This function is valid only when 'p_policy' value is 'FIXED'.
 */
void Scheduler::set_priority()
{
	// if priorities are set, do not change their priorities
	if(p_policy == FIXED && tasks[0]->get_priority() != -1)
	{
		return;
	}

	int cur_priority = 0;
	int min_value;
	int min_index = -1;
	for(unsigned int i = 0; i < tasks.size(); i++)
		tasks[i]->set_priority(-1);
	do
	{
		min_value = MAX_INT;
		min_index = -1;
		for(unsigned int i = 0; i < tasks.size(); i++)
		{
			if(tasks[i]->get_priority() == -1)
			{
				if(p_standard == PERIOD && tasks[i]->get_period() < min_value)
				{
					min_value = tasks[i]->get_period();
					min_index = i;
				}
				else if(p_standard == DEADLINE && tasks[i]->get_a_deadline() < min_value)
				{
					min_value = tasks[i]->get_a_deadline();
					min_index = i;
				}
			}
		}
		if(min_index != -1)
			tasks[min_index]->set_priority(cur_priority);
		cur_priority++;
	} while(min_index != -1);
}

// This function modifies times of events in 'header' liset if current time is greater than stored times
void Scheduler::modify_time(list<Event*> *header, int time)
{
	Event *e;
	while(!header->empty() && header->front()->get_time() < time)
	{
		e = header->front();
		header->pop_front();
		e->set_time(time);
		insert_event(header, e);
	}
}

/* This function extracts the expected schedule between the current time and 'to' of all tasks in the resource who has this scheduler.
 * 
 * <argument>
 * to: end time
 * over_time: time padding
 * plot: the list for plotting
 *
 * <return>
 * 0: unschedulable, 1: schedulable
 */
int Scheduler::extract_schedule(int to, int over_time, list<Time_plot *> *plot)
{
	int end_time = to + over_time;
	if(current_time > end_time)
		return 1;

	list<Event*> h;			// temp list
	unsigned int i;
	int flag = 1;

	// push initial events at the next release time
	for(i = 0; i < tasks.size(); i++)
	{
		Event *e = new Event(START, tasks[i]->get_next_release_time(), tasks[i]->get_wcet(), tasks[i], NULL);
		insert_event(&h, e);
	}

	int stop_condition = 0;
	while(stop_condition != 1)
	{
		modify_time(&h, current_time);

		Event *e = h.front();	// pop one event
		h.pop_front();
		current_time = e->get_time();
		if(e->get_type() == START)		// if start event, create corresponding job and push this event to log
		{
			if(e->task_info_link->get_visible() == 0)		// if dummy, add next release and drop this event
			{
				process_completion(&h, e, end_time, plot);
				delete e;
				continue;
			}
			else
			{
				// create corresponding job
				int task_num = e->task_info_link->get_id();
				e->set_task_id(task_num);
				e->set_job_id(e->task_info_link->get_next_job_id());
				Task *job;
				switch(task_num)
				{
	#include "task_instanciation.hh"

					default:
						printf("task_num: %d\n", task_num);
						job = new Task0(e->task_info_link);
						break;
				}
				e->task_link = job;
				if(e->task_info_link->scheduler_link->get_componentizing() == 1)	// componentizing
					job->set_start_time(job->get_release_time());
				else
					job->set_start_time(current_time);
				e->task_info_link->task_instances.push_back(job);
				log.push_back(e);			// push start log

				/*** internal scheduling behavior ***/
				// push Time_plot for schedule plotting
				add_time_plot(1, current_time, e, plot);
			}
		}
		else if(e->get_type() == RESUMED)
		{
			// push Time_plot for schedule plotting
//			if(e->task_info_link->scheduler_link->get_componentizing() == 0)	// not componentizing
				add_time_plot(1, current_time, e, plot);
		}

		if(!h.empty() && preemptable == PREEMPTABLE)	// preemptable
		{
			list<Event*>::iterator pos;
			Event *temp_e = NULL;
			for(pos = h.begin(); pos != h.end(); pos++)
			{
				if((*pos)->task_info_link->get_priority() < e->task_info_link->get_priority())
				{
					temp_e = *pos;
					break;
				}
			}
			if(temp_e != NULL && (temp_e->get_time() - current_time < e->get_remaining_time()))		// preemption
			{
				Event *temp_event = new Event(RESUMED, temp_e->get_time(), 0, e->task_info_link, e->task_link);
				temp_event->set_remaining_time(e->get_remaining_time() - (temp_e->get_time() - current_time));
				insert_event(&h, temp_event);
				current_time = temp_e->get_time();

				// push Time_plot for schedule plotting
//				if(e->task_info_link->scheduler_link->get_componentizing() == 0)	// not componentizing
					add_time_plot(0, temp_event->get_time(), temp_event, plot);
			}
			else
			{
				current_time += e->get_remaining_time();		// completion
				Event *end = new Event(COMPLETE, current_time, 0, e->task_info_link, e->task_link);
				process_completion(&h, end, end_time, plot);
			}
			if(e->get_type() == RESUMED)
				delete e;
		}
		else		// non-preemptable or there is only one task in this resource
		{
			current_time += e->get_remaining_time();		// completion
			Event *end = new Event(COMPLETE, current_time, 0, e->task_info_link, e->task_link);
			process_completion(&h, end, end_time, plot);
		}

		// stop condition check
		if(current_time > end_time)
		{
			list<Event*>::iterator pos;
			stop_condition = 1;
			for(pos = h.begin(); pos != h.end(); pos++)
			{
				if((*pos)->get_type() == RESUMED)
				{
					stop_condition = 0;
					break;
				}
			}
		}
	}

	while(!h.empty())		// delete all events
	{
		Event *e = h.front();
		h.pop_front();
		delete e;
	}

	if(log.empty())
		return flag;

	// move events from log to log2 for executing in this time window
	int first_task_num = tasks[0]->get_id();		// first task number in this resource
	int num_tasks_in = tasks.size();				// number of tasks in this resource
	vector<int> is_complete(num_tasks_in);
	for(int i = 0; i < num_tasks_in; i++)
		is_complete[i] = 0;
	Event *event_temp = log.front();	// must be start event
	if(event_temp->get_time() > to)
		return flag;
	log.pop_front();
	if(event_temp->get_type() == START)
		is_complete[event_temp->get_task_id()-first_task_num] = 1;
	else
		is_complete[event_temp->get_task_id()-first_task_num] = 0;

	stop_condition = 0;
	while(1)
	{
		log2.push_back(event_temp);
		if(log.front()->get_time() > to)
		{
			stop_condition = 1;
			for(int i = 0; i < num_tasks_in; i++)
			{
				if(is_complete[i] == 1)
				{
					stop_condition = 0;
					break;
				}
			}
			if(stop_condition == 1)
				break;
		}

		event_temp = log.front();
		log.pop_front();
		if(event_temp->get_type() == START)
			is_complete[event_temp->get_task_id()-first_task_num] = 1;
		else
			is_complete[event_temp->get_task_id()-first_task_num] = 0;
	}

	return flag;
}

/* This function processes completion events.
 * 
 * <argument>
 * header: event list
 * e: target event
 * end_time: end time when extracting schedule completes
 * plot: the list for plotting
 */
void Scheduler::process_completion(list<Event*>* header, Event* e, int end_time, list<Time_plot*> *plot)
{
	if(e->task_info_link->get_visible() == 1)
	{
		log.push_back(e);
		current_time = e->get_time();
		if(e->task_info_link->scheduler_link->get_componentizing() == 1)		// componentizing scheduler
			e->task_link->set_completion_time(e->task_link->get_a_deadline());
		else
			e->task_link->set_completion_time(current_time);

		/*** internal scheduling behavior ***/
		// push Time_plot for schedule plotting
//		add_time_plot(0, e->task_link->get_completion_time(), e, plot);
		add_time_plot(0, current_time, e, plot);

		if(current_time > e->task_link->get_a_deadline())		// deadline miss, skip next job
		{
			printf("time %d: deadline miss at task %d\n", current_time, e->task_info_link->get_id());
			e->task_info_link->job_id_increase();
		}
	}

	e->task_info_link->job_id_increase();
	Event *next_release = new Event(START, e->task_info_link->get_next_release_time(), e->task_info_link->get_wcet(), e->task_info_link, NULL);
	insert_event(header, next_release);

	if(p_policy == DYNAMIC)		// dynamic
	{
		set_priority();
		resort(header);
	}
}

/* This function adds Time_plot class for an Event class.
 * In this function, all Time_plot class is sorted in time order.
 * 
 * <argument>
 * is_start: whether this event e is a start event or not
 * time: time for this event
 * e: target event
 * plot: list for Time_plot class
 */
void Scheduler::add_time_plot(int is_start, unsigned long long time, Event *e, list<Time_plot *> *plot)
{
	Time_plot *t = new Time_plot(is_start, time, e->get_task_id(), e->task_info_link->get_task_name(), e->task_info_link->scheduler_link->resource_link->get_resource_name());
	if(plot->empty())
	{
		plot->push_back(t);
	}
	else
	{
		int flag = 0;
		list<Time_plot*>::iterator pos;
		for(pos = plot->begin(); pos != plot->end(); pos++)
		{
			if((*pos)->get_time() > t->get_time())
			{
				plot->insert(pos, t);
				flag = 1;
				break;
			}
		}
		if(flag == 0)
			plot->push_back(t);		// push target to the last position
	}
}
