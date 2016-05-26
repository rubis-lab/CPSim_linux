#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_RESOURCES 10
#define MAX_TASKS 20
#define MAX_VARS 100

using namespace std;

enum {RM, DM, EDF, CPS};

int parsing_hxml();
void generate_code_for_simulator();
void generate_code_for_ecu(char *ecu_id);

char resource_id[MAX_RESOURCES][100];
int is_real[MAX_RESOURCES];
int m_clock[MAX_RESOURCES];
int policy[MAX_RESOURCES];
char task_id[MAX_TASKS][100];		// for each task's code
char send_to[MAX_TASKS][100];		// for data dependency (write)
char recv_from[MAX_TASKS][100];		// for data dependency (read)
int visible[MAX_TASKS];
int period[MAX_TASKS];
int deadline[MAX_TASKS];
int wcet[MAX_TASKS];
int phase[MAX_TASKS];
int connected_ecu[MAX_TASKS];
int num_resources = 0;
int num_tasks = 0;

struct can_read{
	char can_read_id[100];
	int num_vars;
	char var_name1[100];
	char var_name2[100];
};

struct can_read can_read[MAX_VARS];
int num_can_read = 0;					// # of variables

int main(int argc, char* argv[])
{
	// generate initialize.h
	int re = parsing_hxml();
	if(re != 0) exit(re);

	if(argc == 1) generate_code_for_simulator();
	else generate_code_for_ecu(argv[1]);

	return 0;
}

int parsing_hxml()
{
	for(int i = 0; i < MAX_TASKS; i++)
	{
		visible[i] = 1;
	}

	DIR *dir;
	class dirent *ent;
	string fname;
	bool didFind=false;

	dir = opendir(".");
	while((ent=readdir(dir)) != NULL)
	{
		fname = ent->d_name;
	
		if(ent->d_type == DT_DIR || fname[0] == '.') continue;
		else if(strstr(fname.c_str(), ".hxml") != NULL) { didFind=true; break; }
	}
	closedir(dir);

	if(!didFind)
	{
		printf("target hxml file is not found.\n");
		return 3;
	}

	FILE *fp = fopen(fname.c_str(), "r");
	char line[1000];
	char *tok;
	const char *delim = " \t\n<>\"=";
	fgets(line, 1000, fp);		// xml info.
	while(fgets(line, 1000, fp) != NULL)
	{
		if(strstr(line, "CAN id") != NULL)
		{
/*
			while(strstr(line, "/CAN") == NULL)
			{
				fgets(line, 1000, fp);
				if(strstr(line, "<Bandwidth") != NULL)
				{
					fgets(line, 1000, fp);
					tok = strtok(line, delim);	// max
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// kbps
					tok = strtok(NULL, delim);	// value (default: 500)
					target_can.bandwidth = atoi(tok);
				}
				else if(strstr(line, "<Version") != NULL)
				{
					tok = strtok(line, delim);	// Version
					tok = strtok(NULL, delim);	// value (default: 2.A)
					strcpy(target_can.version, tok);
				}
				else if(strstr(line, "<CAN_DB") != NULL)
				{
					tok = strtok(line, delim);	// CAN_DB
					tok = strtok(NULL, ",<");	// value (default: 2.A)
					strcpy(target_can.channel_name[0], tok);
					tok = strtok(NULL, "<");
					if(tok[0] == '/')			// 1 channel
						target_can.num_channel = 1;
					else
					{
						strcpy(target_can.channel_name[1], tok);
						target_can.num_channel = 2;
					}
				}
			}
*/
		}
		else if(strstr(line, "ECU id") != NULL)
		{
			tok = strtok(line, delim);	// ECU
			tok = strtok(NULL, delim);	// id
			tok = strtok(NULL, delim);	// ecu_id
			strcpy(resource_id[num_resources], tok);

			int temp;
			while(strstr(line, "/ECU") == NULL)
			{
				fgets(line, 1000, fp);
				if(strstr(line, "<Virtual") != NULL)
				{
					tok = strtok(line, delim);	// Virtual
					tok = strtok(NULL, delim);	// value (default: 0)
					temp = atoi(tok);
				}
				else if(strstr(line, "<Number") != NULL)
				{
/*
					tok = strtok(line, delim);	// Number_of_cores
					tok = strtok(NULL, delim);	// value (default: 1)
*/
				}
				else if(strstr(line, "<System") != NULL)
				{
					tok = strtok(line, delim);	// SystemClock
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// MHz
					tok = strtok(NULL, delim);	// value (default: 180)
					m_clock[num_resources] = atoi(tok);
				}
				else if(strstr(line, "<Scheduling") != NULL)
				{
					tok = strtok(line, delim);	// SchedulingPolicy
					tok = strtok(NULL, delim);	// value (default: RM)
					if(strcmp(tok, "DM") == 0)
						policy[num_resources] = DM;
					else if(strcmp(tok, "EDF") == 0)
						policy[num_resources] = EDF;
					else if(strcmp(tok, "CPS") == 0)
						policy[num_resources] = CPS;
					else
						policy[num_resources] = RM;
				}
			}
			if(temp == 1)
			{
				num_resources++;
			}
		}
		else if(strstr(line, "SWC id") != NULL)
		{
			tok = strtok(line, delim);	// SWC
			tok = strtok(NULL, delim);	// id
			tok = strtok(NULL, delim);	// swc_id
			int task_id_temp = num_tasks;
			strcpy(task_id[task_id_temp], tok);

			while(strstr(line, "/SWC") == NULL)
			{
				fgets(line, 1000, fp);
				if(strstr(line, "<Connected") != NULL)
				{
					tok = strtok(line, delim);	// Connected_ECU
					tok = strtok(NULL, delim);	// id
					tok = strtok(NULL, delim);	// resource id
					for(int i = 0; i < num_resources; i++)
					{
						if(strcmp(resource_id[i], tok) == 0)		// not virtual
						{
							connected_ecu[task_id_temp] = i;
							num_tasks++;
							break;
						}
					}
				}
				else if(strstr(line, "<Period") != NULL)
				{
					tok = strtok(line, delim);	// Period
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// ms
					tok = strtok(NULL, delim);	// value (default: 0)
					period[task_id_temp] = atoi(tok)*1000;
				}
				else if(strstr(line, "<WCET") != NULL)
				{
					tok = strtok(line, delim);	// WCET
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// ms
					tok = strtok(NULL, delim);	// value (default: 0)
					wcet[task_id_temp] = atoi(tok)*1000;
				}
				else if(strstr(line, "<Phase") != NULL)
				{
					tok = strtok(line, delim);	// Phase
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// ms
					tok = strtok(NULL, delim);	// value (default: 0)
					phase[task_id_temp] = atoi(tok)*1000;
				}
				else if(strstr(line, "<Deadline") != NULL)
				{
					tok = strtok(line, delim);	// Deadline
					tok = strtok(NULL, delim);	// unit
					tok = strtok(NULL, delim);	// ms
					tok = strtok(NULL, delim);	// value (default: 0)
					deadline[task_id_temp] = atoi(tok)*1000;
				}
				else if(strstr(line, "<Send") != NULL)
				{
					tok = strtok(line, delim);	// Send_to
					tok = strtok(NULL, delim);	// value
					strcpy(send_to[task_id_temp], tok);
				}
				else if(strstr(line, "<Recv") != NULL)
				{
					tok = strtok(line, delim);	// Recv_from
					tok = strtok(NULL, delim);	// value
					strcpy(recv_from[task_id_temp], tok);
				}
				else if(strstr(line, "<Visible") != NULL)
				{
					tok = strtok(line, delim);	// Visible
					tok = strtok(NULL, delim);	// value
					visible[task_id_temp] = atoi(tok);
				}
			}
		}
	}
	fclose(fp);
	
	return 0;
}

void generate_code_for_simulator()
{
	char line[1000];
	char *tok;

	FILE *ofp_dummy_h = fopen("dummy.hh", "w");	// for dummy.hh
	FILE *ofp = fopen("initialize.hh", "w");		// for initialize.hh
	FILE *ofp_can_read_h = fopen("can_read.hh", "w");		// for can_read.hh

	FILE *ofp_task_created_h = fopen("task_created.hh", "w");	// for task_created.hh
	fprintf(ofp_task_created_h, "#ifndef __TASKCLASSH_\n#define __TASKCLASSH_\n\n");
	fprintf(ofp_task_created_h, "#include \"stdafx.h\"\n\n");
	fprintf(ofp_task_created_h, "#include \"components.h\"\n\n");
	fprintf(ofp_task_created_h, "// for data from car\nextern float car_output[10];\n\n");
	fprintf(ofp_task_created_h, "extern float user_input[10];\nextern float memory_buffer[10];\n\n");

	FILE *ofp_task_created_c = fopen("task_created.cpp", "w");	// for task_created.cpp
	fprintf(ofp_task_created_c, "#include \"task_created.hh\"\n");
	fprintf(ofp_task_created_c, "#include \"can_api.h\"\n#include \"data_list.hh\"\n\n");
	fprintf(ofp_task_created_c, "extern list<CAN_Msg *> waiting_data;\n\n");

	FILE *ofp_task_instanciation_h = fopen("task_instanciation.hh", "w");
	
	// set resources and schedulers
	fprintf(ofp, "//create resources\n");
	for(int r_id = 0; r_id < num_resources; r_id++)
	{
		fprintf(ofp, "Resource *r%d = new Resource(%d, 1, 100, %d, \"%s\");\n",
			r_id, r_id, m_clock[r_id], resource_id[r_id]);
		fprintf(ofp, "resources.push_back(r%d);\nScheduler *s%d = new Scheduler(%d, 1, r%d);\n",
			r_id, r_id, policy[r_id], r_id);
	}

	// set tasks
	fprintf(ofp, "\n// create tasks\n");
	int is_dummy_write = 0;
	for(int t_id = 0; t_id < num_tasks; t_id++)
	{
		int is_read, is_write, is_dummy;
		int flag;
		/*** set synchronization properties ***/
		// set read sync
		is_read = -1;
		strcpy(line, recv_from[t_id]);
		tok = strtok(line, ",");
		while(tok != NULL)		// for all inputted task id
		{
			flag = 1;
			for(int j = 0; j < num_tasks; j++)
			{
				if(strcmp(tok, task_id[j]) == 0)	// not read sync
				{
					flag = 0;
					break;
				}
			}
			if(flag == 1)		// read sync
			{
				is_read = 1;
				break;
			}
			tok = strtok(NULL, ",");
		}
		if(is_read == -1)		// if not set yet, then not read sync
			is_read = 0;

		// set write sync
		is_write = -1;
		strcpy(line, send_to[t_id]);
		tok = strtok(line, ",");
		while(tok != NULL)		// for all inputted task id
		{
			flag = 1;
			for(int j = 0; j < num_tasks; j++)
			{
				if(strcmp(tok, task_id[j]) == 0)	// not write sync
				{
					flag = 0;
					break;
				}
			}
			if(flag == 1)		// write sync
			{
				is_write = 1;
				break;
			}
			tok = strtok(NULL, ",");
		}
		if(is_write == -1)		// if not set yet, then not write sync
			is_write = 0;

		// if dummy, it's no sync task
		if(strstr(line, "dummy") != NULL)
		{
			is_dummy = 1;
			is_read = 0;
			is_write = 0;
		}

		// for initialize.hh
		fprintf(ofp, "Task_info *t%d = new Task_info(%d, 0, %d, %d, %d, %d, %d, %d, %d, %d, -1, \"%s\", s%d);\n",
			t_id, t_id, period[t_id], wcet[t_id], wcet[t_id]/5, phase[t_id], visible[t_id], is_read, is_write, deadline[t_id], task_id[t_id], connected_ecu[t_id]);
		fprintf(ofp, "whole_tasks.push_back(t%d);\n", t_id);

		/*********************/
		// for task_created.h
		char file_name[20];
		char line_temp[1000];
		char input_var[10][20];
		char output_var[10][20];
		int num_var;
		sprintf(file_name, "%s.h", task_id[t_id]);
		FILE *ifp = fopen(file_name, "r");
		const char *delim_temp = ":\n";

		fprintf(ofp_task_created_h, "class Task%d : public Task\t\t// %s\n{\n", t_id, task_id[t_id]);
		fprintf(ofp_task_created_h, "private:\n");		// private
		fgets(line_temp, 1000, ifp);	// var number
		tok = strtok(line_temp, delim_temp);	// var
		tok = strtok(NULL, delim_temp);			// number of vars
		num_var = atoi(tok);
		for(int i = 0; i < num_var; i++)
		{
			fgets(line_temp, 1000, ifp);
			fprintf(ofp_task_created_h, "\t%s", line_temp);
		}
		fprintf(ofp_task_created_h, "\n");
		while(strstr(line_temp, "input:") == NULL)
			fgets(line_temp, 1000, ifp);

		fprintf(ofp_task_created_h, "public:\n");		// public
		fprintf(ofp_task_created_h, "\tTask%d();\n\tTask%d(Task_info*);\n\t~Task%d();\n\n",
			t_id, t_id, t_id);
		fprintf(ofp_task_created_h, "\tvirtual void read();\n\tvirtual void procedure();\n\tvirtual void write();\n};\n\n");

		/*********************/
		// for task_created.cpp
		fprintf(ofp_task_created_c, "// Task%d (%s)\n", t_id, task_id[t_id]);
		fprintf(ofp_task_created_c, "Task%d::Task%d(Task_info *task_info):Task(task_info)\n{\n}\n\n",
			t_id, t_id);
		fprintf(ofp_task_created_c, "Task%d::~Task%d()\n{\n}\n\n", t_id, t_id);

		// for read() function
		fprintf(ofp_task_created_c, "void Task%d::read()\n{\n", t_id);
		tok = strtok(line_temp, delim_temp);	// input
		tok = strtok(NULL, delim_temp);	// number of inputs
		num_var = atoi(tok);
		if(num_var == 0)
			fprintf(ofp_task_created_c, "\t;\n");
		else
		{
			for(int i = 0; i < num_var; i++)
			{
				fgets(line_temp, 1000, ifp);
				line_temp[strlen(line_temp)-1] = '\0';
				strcpy(input_var[i], line_temp);
				fprintf(ofp_task_created_c, "\t%s = memory_buffer[%d];\n", input_var[i], i);
			}
		}
		fprintf(ofp_task_created_c, "}\n\n");
		while(strstr(line_temp, "output:") == NULL)
			fgets(line_temp, 1000, ifp);

		// for write() function
		fprintf(ofp_task_created_c, "void Task%d::write()\n{\n", t_id);
		tok = strtok(line_temp, delim_temp);	// output
		tok = strtok(NULL, delim_temp);	// number of outputs
		num_var = atoi(tok);
		if(num_var == 0)
			fprintf(ofp_task_created_c, "\t;\n");
		else
		{
			for(int i = 0; i < num_var; i++)
			{
				fgets(line_temp, 1000, ifp);
				line_temp[strlen(line_temp)-1] = '\0';
				strcpy(output_var[i], line_temp);
				fprintf(ofp_task_created_c, "\tmemory_buffer[%d] = %s;\n", i, output_var[i]);
			}
			fprintf(ofp_task_created_c, "\tfor(unsigned int i = 0; i < successors.size(); i++)\n");
			fprintf(ofp_task_created_c, "\t\tsuccessors[i]->read();\n");
		}
		if(is_read == 1)	// read sync task
		{
			while(strstr(line_temp, "CAN_input:") == NULL)
				fgets(line_temp, 1000, ifp);
			tok = strtok(line_temp, " :\n");		// CAN_input
			tok = strtok(NULL, " :\n");			// number of CAN_input
			int num_can_var = atoi(tok);
			for(int i = 0; i < num_can_var; i++)
			{
				fgets(line_temp, 1000, ifp);		// buffer number:CAN ID:# of var
				tok = strtok(line_temp, " :\n");		// buffer number
				tok = strtok(NULL, " :\n");			// CAN ID
				/******** for can_read *********/
				int can_read_num = -1;
				int is_first = 0;		// whether 'case' is written or not for this CAN id
				for(int j = 0; j < num_can_read; j++)
				{
					if(strcmp(can_read[j].can_read_id, tok) == 0)
					{
						can_read_num = j;
						break;
					}
				}
				if(can_read_num == -1)		// new can message
				{
					can_read_num = num_can_read;
					num_can_read++;
					is_first = 1;
					fprintf(ofp_can_read_h, "case %s:\n", tok);
				}
				strcpy(can_read[can_read_num].can_read_id, tok);
				tok = strtok(NULL, " :\n");			// # of var
				can_read[can_read_num].num_vars = atoi(tok);
				fgets(line_temp, 1000, ifp);		// var1
				strcpy(can_read[can_read_num].var_name1, line_temp);
				can_read[can_read_num].var_name1[strlen(can_read[can_read_num].var_name1)-1] = '\0';
				if(is_first == 1)
					fprintf(ofp_can_read_h, "\tmemcpy(&%s, &(msg.DATA[0]), sizeof(float));\n", can_read[can_read_num].var_name1);
				if(can_read[can_read_num].num_vars > 1)
				{
					fgets(line_temp, 1000, ifp);		// var2
					strcpy(can_read[can_read_num].var_name2, line_temp);
					can_read[can_read_num].var_name2[strlen(can_read[can_read_num].var_name2)-1] = '\0';
					if(is_first == 1)
						fprintf(ofp_can_read_h, "\tmemcpy(&%s, &(msg.DATA[4]), sizeof(float));\n", can_read[can_read_num].var_name2);
				}
				if(is_first == 1)
					fprintf(ofp_can_read_h, "\tbreak;\n\n");
			}
		}
		if(is_write == 1)	// write sync task
		{
			while(strstr(line_temp, "CAN_output:") == NULL)
				fgets(line_temp, 1000, ifp);
			tok = strtok(line_temp, " :\n");		// CAN_output
			tok = strtok(NULL, " :\n");			// number of CAN_output
			int num_can_var = atoi(tok);
//			fprintf(ofp_task_created_c, "\n\tif(task_link->get_is_write() == 1)\n\t{\n");
			fprintf(ofp_task_created_c, "\n\t// can send\n");
			for(int i = 0; i < num_can_var; i++)
			{
				fgets(line_temp, 1000, ifp);		// buffer number:CAN ID:# of var
				tok = strtok(line_temp, " :\n");		// buffer number
				tok = strtok(NULL, " :\n");			// CAN ID
				char can_id[100];
				strcpy(can_id, tok);
				tok = strtok(NULL, " :\n");			// # of var
				int num_var_local = atoi(tok);
				char var1[100], var2[100];
				fgets(var1, 1000, ifp);		// var1
				var1[strlen(var1)-1] = '\0';
				if(num_var_local > 1)
				{
					fgets(var2, 1000, ifp);		// var2
					var2[strlen(var2)-1] = '\0';
					fprintf(ofp_task_created_c, "\tCAN_Msg *can_msg = new CAN_Msg(completion_time, 1, %s, %s, %s, this->task_link->get_task_name());\n", can_id, var1, var2);
				}
				else
					fprintf(ofp_task_created_c, "\tCAN_Msg *can_msg = new CAN_Msg(completion_time, 1, %s, %s, %s, this->task_link->get_task_name());\n", can_id, var1, var1);
					
				fprintf(ofp_task_created_c, "\tinsert_can_msg(&waiting_data, can_msg);\n");
			}
//			fprintf(ofp_task_created_c, "\t}\n");	// end if
		}

		/*** dummy.h ***/
		if(strstr(line, "dummy") != NULL)	// dummy task
		{
			while(strstr(line_temp, "CAN_ID:") == NULL)
				fgets(line_temp, 1000, ifp);
			fgets(line_temp, 1000, ifp);		// CAN ID
			line_temp[strlen(line_temp)-1] = '\0';
			int dummy_id = atoi(task_id[t_id]+5);
			if(is_dummy_write == 0)
			{
				fprintf(ofp_dummy_h, "case %s:\n", line_temp);
				fprintf(ofp_dummy_h, "\tint t_id, on_off;\n");
				fprintf(ofp_dummy_h, "\tmemcpy(&t_id, &(msg.DATA[0]), sizeof(int));\n");
				fprintf(ofp_dummy_h, "\tmemcpy(&on_off, &(msg.DATA[4]), sizeof(int));\n");
				fprintf(ofp_dummy_h, "\tif(t_id == %d)\n", dummy_id);
				is_dummy_write = 1;
			}
			else
				fprintf(ofp_dummy_h, "\telse if(t_id == %d)\n", dummy_id);
			fprintf(ofp_dummy_h, "\t\twhole_tasks[%d]->set_visible(on_off);\n", t_id);
		}
		/****************/

		fprintf(ofp_task_created_c, "}\n\n");
		while(strstr(line_temp, "code:") == NULL)
			fgets(line_temp, 1000, ifp);

		// for procedure() function
		fprintf(ofp_task_created_c, "void Task%d::procedure()\n", t_id);
		while(fgets(line_temp, 1000, ifp) != NULL)
		{
			fprintf(ofp_task_created_c, "%s", line_temp);
		}
		fprintf(ofp_task_created_c, "\n\n");

		/*********************/
		// for task_instanciation.hh
		fprintf(ofp_task_instanciation_h, "case %d:\n", t_id);
		fprintf(ofp_task_instanciation_h, "\tjob = new Task%d(e->task_info_link);\n", t_id);
		fprintf(ofp_task_instanciation_h, "\tbreak;\n\n");
	}

	// set data dependency
	fprintf(ofp, "\n// set data dependency\n");
	for(int t_id = 0; t_id < num_tasks; t_id++)
	{
		strcpy(line, send_to[t_id]);
		if(strstr(line, "dummy") != NULL)		// if dummy task, do nothing
			continue;
		tok = strtok(line, ",");
		while(tok != NULL)
		{
			for(int j = 0; j < num_tasks; j++)
			{
				if(strcmp(tok, task_id[j]) == 0)	// successor
				{
					fprintf(ofp, "t%d->successors.push_back(t%d);\nt%d->predecessors.push_back(t%d);\n", 
						t_id, j, j, t_id);
					break;
				}
			}
			tok = strtok(NULL, ",");
		}
	}

	// for data_list.hh
	FILE *ofp_data_list_h = fopen("data_list.hh", "w");
	char enum_name[100];
	fprintf(ofp_data_list_h, "enum user_input_name{ACCELERATION, BRAKE, STEER};\n");
	fprintf(ofp_data_list_h, "enum car_output_name{");
	if(num_can_read > 0)
	{
		strcpy(enum_name, can_read[0].var_name1);
		tok = strtok(enum_name, " []\n");	// car_output
		tok = strtok(NULL, " []\n");		// for enum
		fprintf(ofp_data_list_h, "%s", tok);
		if(can_read[0].num_vars > 1)
		{
			strcpy(enum_name, can_read[0].var_name2);
			tok = strtok(enum_name, " []\n");	// car_output
			tok = strtok(NULL, " []\n");		// for enum
			fprintf(ofp_data_list_h, ", %s", tok);
		}
	}
	for(int i = 1; i < num_can_read; i++)
	{
		strcpy(enum_name, can_read[i].var_name1);
		tok = strtok(enum_name, " []\n");	// car_output
		tok = strtok(NULL, " []\n");		// for enum
		fprintf(ofp_data_list_h, ", %s", tok);
		if(can_read[i].num_vars > 1)
		{
			strcpy(enum_name, can_read[i].var_name2);
			tok = strtok(enum_name, " []\n");	// car_output
			tok = strtok(NULL, " []\n");		// for enum
			fprintf(ofp_data_list_h, ", %s", tok);
		}
	}
	fprintf(ofp_data_list_h, "};\n");
//	fprintf(ofp_data_list_h, "float car_output[10];\n");
	fclose(ofp_data_list_h);

	fclose(ofp);
	fprintf(ofp_task_created_h, "#endif\n");
	fclose(ofp_task_created_h);
	fclose(ofp_task_created_c);
	fclose(ofp_task_instanciation_h);
	fprintf(ofp_dummy_h, "\tbreak;\n");
	fclose(ofp_dummy_h);
	fclose(ofp_can_read_h);
}

void generate_code_for_ecu(char *ecu_id)
{
	// find corresponding ecu
	int r_id = -1;
	for(int i = 0; i < num_resources; i++)
	{
		if(strcmp(ecu_id, resource_id[i]) == 0)
		{
			r_id = i;
			break;
		}
	}
	if(r_id == -1)
	{
		return;
	}

	int r_policy = policy[r_id];

	// find corresponding task
	int t_id_from, t_id_to;
	for(int i = 0; i < num_tasks; i++)
	{
		if(connected_ecu[i] == r_id)
		{
			t_id_from = i;
			break;
		}
	}
	for(int i = num_tasks-1; i >= 0; i--)
	{
		if(connected_ecu[i] == r_id)
		{
			t_id_to = i;
			break;
		}
	}

	/**** for setting enum ****/
	char file_name[100];
	FILE *fp_algo;
	char line[1000];
	const char *delim = " :\n";
	char *tok;
	for(int i = t_id_from; i <= t_id_to; i++)
	{
		// open algorithm code
		sprintf(file_name, "%s.h", task_id[i]);
		if(strstr(file_name, "dummy") != NULL)		// if dummy, do nothing
			continue;
		fp_algo = fopen(file_name, "r");
		fgets(line, 1000, fp_algo);
		while(strstr(line, "CAN_input:") == NULL)
			fgets(line, 1000, fp_algo);
		tok = strtok(line, " :\n");		// CAN_input
		tok = strtok(NULL, " :\n");			// number of CAN_input
		int num_can_var = atoi(tok);
		for(int k = 0; k < num_can_var; k++)
		{
			fgets(line, 1000, fp_algo);		// buffer number:CAN ID:# of var
			tok = strtok(line, " :\n");		// buffer number
			tok = strtok(NULL, " :\n");			// CAN ID
			/******** for can_read *********/
			int can_read_num = -1;
			for(int j = 0; j < num_can_read; j++)
			{
				if(strcmp(can_read[j].can_read_id, tok) == 0)
				{
					can_read_num = j;
					break;
				}
			}
			if(can_read_num == -1)		// new can message
			{
				can_read_num = num_can_read;
				num_can_read++;
			}
			strcpy(can_read[can_read_num].can_read_id, tok);
			tok = strtok(NULL, " :\n");			// # of var
			can_read[can_read_num].num_vars = atoi(tok);
			fgets(line, 1000, fp_algo);		// var1
			strcpy(can_read[can_read_num].var_name1, line);
			can_read[can_read_num].var_name1[strlen(can_read[can_read_num].var_name1)-1] = '\0';
			if(can_read[can_read_num].num_vars > 1)
			{
				fgets(line, 1000, fp_algo);		// var2
				strcpy(can_read[can_read_num].var_name2, line);
				can_read[can_read_num].var_name2[strlen(can_read[can_read_num].var_name2)-1] = '\0';
			}
		}
		fclose(fp_algo);
	}
	/******************/

	FILE *fp_c = fopen("user_main.c", "w");
	char temp[1000];

	fprintf(fp_c, "#include \"user_main.h\"\n\n");

	fprintf(fp_c, "enum user_input_name{ACCELERATION, BRAKE, STEER};\n");
//	fprintf(fp_c, "enum car_output_name{CC_TRIGGER, LKAS_TRIGGER, SPEED, STEER_VALUE, RPM, YAW_RATE, DISTANCE, PASSED_TIME, TARGET_SPEED, ACCEL_VALUE, TARGET_RPM, IS_RUNNING, WHEEL_SPEED, TARGET_WHEEL_SPEED};\n\n");
	char enum_name[100];
	fprintf(fp_c, "enum car_output_name{");
	if(num_can_read > 0)
	{
		strcpy(enum_name, can_read[0].var_name1);
		tok = strtok(enum_name, " []\n");	// car_output
		tok = strtok(NULL, " []\n");		// for enum
		fprintf(fp_c, "%s", tok);
		if(can_read[0].num_vars > 1)
		{
			strcpy(enum_name, can_read[0].var_name2);
			tok = strtok(enum_name, " []\n");	// car_output
			tok = strtok(NULL, " []\n");		// for enum
			fprintf(fp_c, ", %s", tok);
		}
	}
	for(int i = 1; i < num_can_read; i++)
	{
		strcpy(enum_name, can_read[i].var_name1);
		tok = strtok(enum_name, " []\n");	// car_output
		tok = strtok(NULL, " []\n");		// for enum
		fprintf(fp_c, ", %s", tok);
		if(can_read[i].num_vars > 1)
		{
			strcpy(enum_name, can_read[i].var_name2);
			tok = strtok(enum_name, " []\n");	// car_output
			tok = strtok(NULL, " []\n");		// for enum
			fprintf(fp_c, ", %s", tok);
		}
	}
	fprintf(fp_c, "};\n\n");

	// task declaration
	for(int i = t_id_from; i <= t_id_to; i++)
		fprintf(fp_c, "static void func_%s(void *arg);\n", task_id[i]);
	fprintf(fp_c, "\nvoid func_idle(void *arg);\n\n");		// idle thread
	fprintf(fp_c, "#pragma align 8\n\n");

	// declare HWCs
	if(r_policy != RM)	// CPS kernel
	{
		for(int i = t_id_from; i <= t_id_to; i++)
			fprintf(fp_c, "DECLARE_HWC(hwc_%d,\t%d,\t%d);\n", i-t_id_from+1, period[i]/1000, wcet[i]/1000);
	}
	else
		fprintf(fp_c, "DECLARE_HWC(hwc_1,\t10,\t10);\n");
	fprintf(fp_c, "DECLARE_HWC(gnd,\t0,\t0);\t\t// ground component\n");
	fprintf(fp_c, "\n");

	// declare SWCs
	if(r_policy != RM)	// CPS kernel
	{
		for(int i = t_id_from; i <= t_id_to; i++)
		{
			fprintf(fp_c, "DECLARE_SWC(swc_%s,\t1,\t%d,\tDEFAULT_STACK_SIZE,", task_id[i], period[i]/1000);
			if(visible[i] == 1)
				fprintf(fp_c, "\tACTIVE)\n");
			else
				fprintf(fp_c, "\tINACTIVE)\n");
			fprintf(fp_c, "DECLARE_SWC(idle_%d,\t0,\t0,\tDEFAULT_STACK_SIZE,\tACTIVE)\n\n", i-t_id_from+1);
		}
	}
	else
	{
		fprintf(fp_c, "DECLARE_SWC(idle,\t0,\t0,\tDEFAULT_STACK_SIZE,\tACTIVE)\n\n");
		for(int i = t_id_from; i <= t_id_to; i++)
		{
			int pri = 1;	// min priority
			for(int j = t_id_from; j <= t_id_to; j++)
			{
				if(period[j] > period[i])	// rate monotonic
					pri++;
			}
			fprintf(fp_c, "DECLARE_SWC(swc_%s,\t%d,\t%d,\tDEFAULT_STACK_SIZE,", task_id[i], pri, period[i]/1000);
			if(visible[i] == 1)
				fprintf(fp_c, "\tACTIVE)\n");
			else
				fprintf(fp_c, "\tINACTIVE)\n");
		}
	}
	fprintf(fp_c, "DECLARE_SWC(gnd_idle,\t0,\t0,\tDEFAULT_STACK_SIZE,\tACTIVE)\n\n");
	fprintf(fp_c, "#pragma align restore\n\n");
	fprintf(fp_c, "float internal_data[10];\t// for internal comm.\n\n");		// for internal comm.

	// callback function for dynamic load injection
	fprintf(fp_c, "void callback(uint32_t swc_id, uint32_t flag)\n{");
	fprintf(fp_c, "\tswitch (swc_id) {\n");
	int case_num = 0;
	for(int i = t_id_from; i <= t_id_to; i++)
	{
		if(strstr(task_id[i], "dummy") != NULL)
		{
			case_num++;
			fprintf(fp_c, "\t\tcase %d:\n\t\t\tswc_%s->active = flag;\n\t\t\tbreak;\n", case_num, task_id[i]);
		}
	}
	fprintf(fp_c, "\t}\n}\n\n");
/*
	// component control blocks
	for(int i = t_id_from; i <= t_id_to; i++)
		fprintf(fp_c, "COMPONENT_CONTROL_BLOCK(component_%d,\t%d,\t%d);\n", i-t_id_from+1, period[i]/1000, wcet[i]/1000);
	fprintf(fp_c, "COMPONENT_CONTROL_BLOCK(gnd,\t0,\t0);\t\t// ground component\n");
	fprintf(fp_c, "\n");

	// task control blocks
	for(int i = t_id_from; i <= t_id_to; i++)
	{
		fprintf(fp_c, "TASK_CONTROL_BLOCK(task_%s,\t1,\t%d,\tDEFAULT_STACK_SIZE)\n", task_id[i], period[i]/1000);
		fprintf(fp_c, "TASK_CONTROL_BLOCK(idle_%d,\t0,\t0,\tDEFAULT_STACK_SIZE)\n\n", i-t_id_from+1);
	}
	fprintf(fp_c, "TASK_CONTROL_BLOCK(gnd_task_idle,\t0,\t0,\tDEFAULT_STACK_SIZE)\n\n");
	fprintf(fp_c, "#pragma align restore\n\n");
	fprintf(fp_c, "float internal_data[10];\t// for internal comm.\n\n");		// for internal comm.
*/
	/*
	for(int i = 0; i < num_tasks; i++)		// dummy load
	{
		if(strstr(task_id[i], "dummy") != NULL)
			fprintf(fp_c, "int %s_load = %d;\n", task_id[i], visible[i]);
	}
	fprintf(fp_c, "\n");
	*/

	// code for user_main(void)
	fprintf(fp_c, "void user_main(void)\n{\n");
	fprintf(fp_c, "\tprintf(\"Starting HW/SW Components...\\n\");\n\n");
	if(r_policy != RM)		// CPS kernel
	{
		for(int i = t_id_from; i <= t_id_to; i++)
		{
			fprintf(fp_c, "\tcreate_hwc(hwc_%d);\n", i-t_id_from+1);
			fprintf(fp_c, "\tcreate_swc(hwc_%d, swc_%s, NULL, func_%s, WORK);\n", i-t_id_from+1, task_id[i], task_id[i]);
			fprintf(fp_c, "\tcreate_swc(hwc_%d, idle_%d, NULL, func_idle, IDLE);\n", i-t_id_from+1, i-t_id_from+1);
		}
	}
	else
	{
		fprintf(fp_c, "\tcreate_hwc(hwc_1);\n");
		for(int i = t_id_from; i <= t_id_to; i++)
			fprintf(fp_c, "\tcreate_swc(hwc_1, swc_%s, NULL, func_%s, WORK);\n", task_id[i], task_id[i]);
		fprintf(fp_c, "\tcreate_swc(hwc_1, idle, NULL, func_idle, IDLE);\n");
	}
	fprintf(fp_c, "\n\tcreate_hwc(gnd);\n");
	fprintf(fp_c, "\tcreate_swc(gnd, gnd_idle, NULL, func_idle, IDLE);\n\n");

	// timer
	fprintf(fp_c, "\t/* Enable timer interrupt */\n");
	fprintf(fp_c, "\tprintf(\"Starting The Timer Interrupt...\\n\");\n");
	fprintf(fp_c, "\tinit_timer_interrupt();\n");
	fprintf(fp_c, "\tset_current_hwc(hwc_1);\n\n");

	// schedule
	fprintf(fp_c, "\t/* only the idle swc is running after calling schedule() and before the 1st timer interrupt */\n");
	fprintf(fp_c, "\tprintf(\"Starting The Scheduler...\\n\");\n");
	fprintf(fp_c, "\tschedule();\n}\n\n");

	// code for each task
	for(int i = t_id_from; i <= t_id_to; i++)
	{
		// open algorithm code
		sprintf(file_name, "%s.h", task_id[i]);
		fp_algo = fopen(file_name, "r");

		fprintf(fp_c, "static void func_%s(void *arg)\n{\n", task_id[i]);
		fgets(line, 1000, fp_algo);

		if(strstr(task_id[i], "dummy") != NULL)		// dummy task
		{
			// dummy code
			fprintf(fp_c, "\n\twhile (1) {\n");
			fprintf(fp_c, "\t\tif (self()->active) {\n");
			fprintf(fp_c, "\t\t\tdelay_ms_f(%d);\n\t\t}\n", wcet[i]/1000);
			fprintf(fp_c, "\t\tsyscall_suspend();\n\t}\n}\n\n");
			fclose(fp_algo);
		}
		else		// general task (not dummy)
		{
			// var
			while(strstr(line, "var:") == NULL)
				fgets(line, 1000, fp_algo);
			tok = strtok(line, delim);
			tok = strtok(NULL, delim);
			int num = atoi(tok);
			for(int j = 0; j < num; j++)
			{
				fgets(line, 1000, fp_algo);
				fprintf(fp_c, "\t%s", line);
			}
			fprintf(fp_c, "\tfloat car_output[10];\n");
			fprintf(fp_c, "\tfloat user_input[10];\n\n");

			// CAN_input
			while(strstr(line, "CAN_input:") == NULL)
				fgets(line, 1000, fp_algo);
			tok = strtok(line, delim);
			tok = strtok(NULL, delim);
			num = atoi(tok);
			for(int j = 0; j < num; j++)
				fprintf(fp_c, "\tCAN_SWObj sensing%d;\n", j);
			fprintf(fp_c, "\tubyte send_buf[16];\n\n");

			// while
			fprintf(fp_c, "\twhile (1) {\n");
			for(int j = 0; j < num; j++)
			{
				fgets(line, 1000, fp_algo);
				tok = strtok(line, delim);
				int buffer_id = atoi(tok);
				tok = strtok(NULL, delim);	// CAN id
				tok = strtok(NULL, delim);	// # of vars
				int num_var_local = atoi(tok);
				fprintf(fp_c, "\t\tCAN_vGetMsgObj(%d, &sensing%d);\n", buffer_id, j);
				fprintf(fp_c, "\t\tCAN_vReleaseObj(%d);\n", buffer_id);
				fgets(line, 1000, fp_algo);
				line[strlen(line)-1] = '\0';
				fprintf(fp_c, "\t\t%s = *((float *)&sensing%d.ubData[0]);\n", line, j);
				if(num_var_local > 1)
				{
					fgets(line, 1000, fp_algo);
					line[strlen(line)-1] = '\0';
					fprintf(fp_c, "\t\t%s = *((float *)&sensing%d.ubData[4]);\n\n", line, j);
				}
			}

			// input
			fp_algo = fopen(file_name, "r");
			fgets(line, 1000, fp_algo);
			while(strstr(line, "input:") == NULL)
				fgets(line, 1000, fp_algo);
			tok = strtok(line, delim);
			tok = strtok(NULL, delim);
			num = atoi(tok);
			for(int j = 0; j < num; j++)
			{
				fgets(line, 1000, fp_algo);
				line[strlen(line)-1] = '\0';
				fprintf(fp_c, "\t\t%s = internal_data[%d];\n", line, j);
			}
			fprintf(fp_c, "\n");

			// code
			while(strstr(line, "code:") == NULL)
				fgets(line, 1000, fp_algo);
			while(fgets(line, 1000, fp_algo) != NULL)
				fprintf(fp_c, "\t\t%s", line);
			fclose(fp_algo);

			// output
			fp_algo = fopen(file_name, "r");
			fgets(line, 1000, fp_algo);
			while(strstr(line, "output:") == NULL)
				fgets(line, 1000, fp_algo);
			tok = strtok(line, delim);
			tok = strtok(NULL, delim);
			num = atoi(tok);

			// internal data communication
			for(int j = 0; j < num; j++)
			{
				fgets(line, 1000, fp_algo);
				line[strlen(line)-1] = '\0';
				fprintf(fp_c, "\t\tinternal_data[%d] = %s;\n", j, line);
			}
			fprintf(fp_c, "\n");

			// CAN_output
			fgets(line, 1000, fp_algo);
			while(strstr(line, "CAN_output:") == NULL)
				fgets(line, 1000, fp_algo);
			tok = strtok(line, delim);
			tok = strtok(NULL, delim);
			num = atoi(tok);
			for(int j = 0; j < num; j++)
			{
				fgets(line, 1000, fp_algo);
				tok = strtok(line, delim);
				int buffer_id = atoi(tok);
				tok = strtok(NULL, delim);	// CAN id
				tok = strtok(NULL, delim);	// # of vars
				int num_var_local = atoi(tok);
				fgets(line, 1000, fp_algo);
				line[strlen(line)-1] = '\0';
				fprintf(fp_c, "\t\tmemcpy(&send_buf[0], &%s, 4);\n", line);
				if(num_var_local > 1)
				{
					fgets(line, 1000, fp_algo);
					line[strlen(line)-1] = '\0';
					fprintf(fp_c, "\t\tmemcpy(&send_buf[4], &%s, 4);\n", line);
				}
				fprintf(fp_c, "\t\tCAN_vLoadData(%d, (ubyte *)send_buf);\n", buffer_id);
				if(r_policy != RM)
					fprintf(fp_c, "\n\t\tdelayed_output(%d, self()->timer);\n", buffer_id);
				else
				{
					if(wcet[i]/1000 > 5)
						fprintf(fp_c, "\n\t\tdelay_ms_f(%d);\n", wcet[i]/1000-5);
					fprintf(fp_c, "\t\tCAN_vTransmit(%d);\n", buffer_id);
				}
			}
			fprintf(fp_c, "\n\t\tsyscall_suspend();\n\t}\n}\n\n");
			fclose(fp_algo);
		}
	}

	// code for idle task
	fprintf(fp_c, "void func_idle(void* arg)\n{\n\tfor (;;) {\n");
	fprintf(fp_c, "\t\t__nop();\n\t}\n}\n\n");

	fclose(fp_c);
}
