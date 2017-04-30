#include "components.h"

Task_info::Task_info()
{
	priority = -1;
}

Task_info::Task_info(int id_input, int type_input, int hw_id_input, int period_input, int bcet_input, int wcet_input,
	int modified_rate_input, int phase_input, int visible_input, int read_input, int write_input, int dead_input, int pri,
	char *name):Component(id_input)
{
	type = type_input;
	hw_id = hw_id_input;
	period = period_input;
	bcet_ECU = bcet_input;
	wcet_ECU = wcet_input;
	modified_rate = modified_rate_input;
	bcet_PC = bcet_ECU*modified_rate/100;
	wcet_PC = wcet_ECU*modified_rate/100;
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
//	scheduler_link = scheduler;
//	scheduler->tasks.push_back(this);
}

Task_info::~Task_info()
{
}

// after a job is created, next job_id and release time should be modified for next job creation
void Task_info::job_id_increase()
{
	next_job_id++;
	next_release_time += period;
	a_deadline = next_release_time + r_deadline;
}
