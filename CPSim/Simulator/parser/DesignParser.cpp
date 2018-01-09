#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <string.h>

using namespace std;

#include <libxml/xmlreader.h>

#ifdef LIBXML_READER_ENABLED

#define MAX_TASKS 100
#define MAX_VARS 100
#define LOCATION "../engine/auto_generated_files/"

class DesignParser {
private:
	map<string,int> ecumap;
    char task_id[MAX_TASKS][100];		// for each task's code
    char send_to[MAX_TASKS][100];		// for data dependency (write)
    int num_tasks;
	int getEcuIdx(char* ecuname);
	void parseECUs(xmlDocPtr doc, xmlNodePtr cur);
	void parseSWCs(xmlDocPtr doc, xmlNodePtr cur);
	void generateTaskCode();

    struct can_read{
    	char can_read_id[100];
    	int num_vars;
    	char var_name1[100];
    	char var_name2[100];
    };

    struct can_read can_read[MAX_VARS];
    int num_can_read;					// # of variables

public:
	void getSource();
	void getCharProp(xmlNodePtr cur, const char* prop, char* dest);
};

void DesignParser::getCharProp(xmlNodePtr cur, const char* prop, char* dest) {
	xmlChar* e;
	e = xmlGetProp(cur, (const xmlChar*)prop);
	strcpy(dest, (char*)e);
	xmlFree(e);
}

int DesignParser::getEcuIdx(char* ecuname) {
	int ret=-1;
	if(ecumap.find(ecuname) != ecumap.end()) {
		ret = ecumap[ecuname];
	}
	return ret;
}

void DesignParser::parseECUs(xmlDocPtr doc, xmlNodePtr cur) {
//	FILE* fp = fopen("../Simulator/auto_generated_files/ecudec.hh", "w");
    char file_name[100];
    sprintf(file_name, "%secudec.hh", LOCATION);
	FILE* fp = fopen(file_name, "w");
	int cnt=0;
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		char id[100], v[5], sched_policy[10];
		if((!xmlStrcmp(cur->name, (const xmlChar*)"ECU"))) {
			bool isVirtual = false;

			getCharProp(cur, "ID", id);
			getCharProp(cur, "Virtual", v);
			getCharProp(cur, "schedPolicy", sched_policy);

			if(strncmp(v, "1", 1) == 0) isVirtual = true;
			if(isVirtual) {
				ecumap.insert(make_pair(id, ecumap.size()));
				fprintf(fp, "Resource *r%d = new Resource(%d, 1, 100, 100, \"%s\");\n", cnt, cnt, id);
				fprintf(fp, "resources.push_back(r%d);\n", cnt);
				fprintf(fp, "Scheduler *s%d = new Scheduler(%s, 1, r%d);\n", cnt, sched_policy, cnt);
				cnt++;
			}
		}
		cur = cur->next;
	}
	fclose(fp);
}

void DesignParser::parseSWCs(xmlDocPtr doc, xmlNodePtr cur) {
//	FILE* fp = fopen("../Simulator/auto_generated_files/swcdec.hh", "w");
    char file_name[100];
    sprintf(file_name, "%sswcdec.hh", LOCATION);
	FILE* fp = fopen(file_name, "w");
	num_tasks = 0;
	cur = cur->xmlChildrenNode;
	int cnt=0;
	int swcidx = 0;
	while(cur != NULL) {
		char id[100], period[10], wcet[10], deadline[10], phase[10], ecuid[100], v[5], sendto[100];
		if((!xmlStrcmp(cur->name, (const xmlChar*)"SWC"))) {
			xmlNodePtr lcur = cur->xmlChildrenNode;
			while(lcur != NULL) {
				if((!xmlStrcmp(lcur->name, (const xmlChar*)"link"))) {
					getCharProp(lcur, "rid", ecuid);
					break;
				}
				lcur = lcur->next;
			}
			bool isVirtual = false;
			
			getCharProp(cur, "ID", id);
			getCharProp(cur, "deadline", deadline);	// dealine
			getCharProp(cur, "period", period);     // period
			getCharProp(cur, "WCET", wcet);		// WCET
			getCharProp(cur, "phase", phase);   // phase
			getCharProp(cur, "Virtual", v);
			getCharProp(cur, "sendto", sendto);     // sendto
			strcpy(task_id[swcidx], id);
			strcpy(send_to[swcidx], sendto);
			
			if(strncmp(v, "1", 1) == 0) isVirtual = true;
			if(isVirtual) {
				int ecuidx = getEcuIdx(ecuid);
				if(ecuidx >= 0) {
					fprintf(fp, "Task_info *t%d = new Task_info(%d, 0, %d, %d, %d, %d, 1, 1, 1, %d, -1, \"%s\", s%d);\n", swcidx, swcidx, atoi(period)*1000, atoi(wcet)*1000, atoi(wcet)/5*1000, atoi(phase)*1000, atoi(deadline)*1000, id, ecuidx);
					fprintf(fp, "whole_tasks.push_back(t%d);\n", swcidx);
					swcidx++;
					num_tasks++;
				}
			}
		}
		cur = cur->next;
	}

	// set data dependency
	fprintf(fp, "\n//data dependency\n");
	char *tok;
	num_tasks = swcidx;
	for(int i = 0; i < num_tasks; i++)
    {
        if(strlen(send_to[i]) == 0)
            continue;
        tok = strtok(send_to[i], ",");
        do
        {
            for(int j = 0; j < num_tasks; j++)
            {
                if(strcmp(task_id[j], tok) == 0)
                {
				    fprintf(fp, "t%d->successors.push_back(t%d);\nt%d->predecessors.push_back(t%d);\n", 
						i, j, j, i);
                    break;
                }
            }
        } while(tok = strtok(NULL, ","));
    }
	fclose(fp);
}

void DesignParser::generateTaskCode()
{
    char file_name[100];
    sprintf(file_name, "%scan_read.hh", LOCATION);
	FILE *ofp_can_read_h = fopen(file_name, "w");		// for can_read.hh
    
    sprintf(file_name, "%stask_created.hh", LOCATION);
    FILE *ofp_task_created_h = fopen(file_name, "w");
	fprintf(ofp_task_created_h, "#ifndef __TASKCLASSH_\n#define __TASKCLASSH_\n\n");
	fprintf(ofp_task_created_h, "#include \"components.h\"\n\n");
	fprintf(ofp_task_created_h, "// for data from car\n#ifdef NOCANMODE\nextern float *car_output;\n#else\nextern float car_output[10];\n#endif\n");
	fprintf(ofp_task_created_h, "extern float memory_buffer[10];\n\n");
    
    sprintf(file_name, "%stask_created.cpp", LOCATION);
    FILE *ofp_task_created_c = fopen(file_name, "w");
	fprintf(ofp_task_created_c, "#include \"task_created.hh\"\n");
	fprintf(ofp_task_created_c, "#include \"can_api.h\"\n#include \"data_list.hh\"\n\n");
	fprintf(ofp_task_created_c, "extern list<CAN_Msg *> waiting_data;\n\n");
	
    sprintf(file_name, "%stask_instanciation.hh", LOCATION);
	FILE *ofp_task_instanciation_h = fopen(file_name, "w");

	char *tok;
	int num_can_read = 0;
    for(int t_id = 0; t_id < num_tasks; t_id++)
    {
		// for task_created.h
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

		fprintf(ofp_task_created_h, "public:\n");		// public
		fprintf(ofp_task_created_h, "\tTask%d();\n\tTask%d(Task_info*);\n\t~Task%d();\n\n",
			t_id, t_id, t_id);
		fprintf(ofp_task_created_h, "\tvirtual void procedure();\n\tvirtual void write();\n};\n\n");

		/*********************/
		// for task_created.cpp
		fprintf(ofp_task_created_c, "// Task%d (%s)\n", t_id, task_id[t_id]);
		fprintf(ofp_task_created_c, "Task%d::Task%d(Task_info *task_info):Task(task_info)\n{\n}\n\n",
			t_id, t_id);
		fprintf(ofp_task_created_c, "Task%d::~Task%d()\n{\n}\n\n", t_id, t_id);

		// for write() function
		while(strstr(line_temp, "output:")  == NULL)
		    fgets(line_temp, 1000, ifp);
		fprintf(ofp_task_created_c, "void Task%d::write()\n{\n", t_id);
		tok = strtok(line_temp, delim_temp);	// output
		tok = strtok(NULL, delim_temp);	// number of outputs
		num_var = atoi(tok);
		if(num_var == 0)
			fprintf(ofp_task_created_c, "\t;\n");
		else
		{
			fprintf(ofp_task_created_c, "\tfor(unsigned int i = 0; i < successors.size(); i++)\n\t{\n");
			for(int i = 0; i < num_var; i++)
			{
				fgets(line_temp, 1000, ifp);
				line_temp[strlen(line_temp)-1] = '\0';
				strcpy(output_var[i], line_temp);
				fprintf(ofp_task_created_c, "\t\tsuccessors[i]->%s = %s;\n", output_var[i], output_var[i]);
			}
			fprintf(ofp_task_created_c, "\t}\n");
			
		}

        while(strstr(line_temp, "CAN_input:") == NULL)
			fgets(line_temp, 1000, ifp);
		tok = strtok(line_temp, " :\n");		// CAN_input
		tok = strtok(NULL, " :\n");			// number of CAN_input
		int num_can_var = atoi(tok);
		if(num_can_var > 0)	// read sync task
		{
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

        while(strstr(line_temp, "CAN_output:") == NULL)
			fgets(line_temp, 1000, ifp);
		tok = strtok(line_temp, " :\n");		// CAN_output
		tok = strtok(NULL, " :\n");			// number of CAN_output
		num_can_var = atoi(tok);
		if(num_can_var > 0)	// write sync task
		{
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
				char var1[100], var1_index[100], var2[100], var2_index[100];
				fgets(line_temp, 1000, ifp);		// var1 and index
				tok = strtok(line_temp, " \n");
				strcpy(var1, tok);
				strcpy(var2, tok);
				tok = strtok(NULL, " \n");
				strcpy(var1_index, tok);
				strcpy(var2_index, tok);
				if(num_var_local > 1)
				{
					fgets(line_temp, 1000, ifp);		// var2 and index
			    	tok = strtok(line_temp, " \n");
			    	strcpy(var2, tok);
			    	tok = strtok(NULL, " \n");
			    	strcpy(var2_index, tok);
				}
				fprintf(ofp_task_created_c, "\tCAN_Msg *can_msg = new CAN_Msg(completion_time, 1, %s, %d, %s, %s, %s, %s, this->task_link->get_task_name());\n", can_id, num_var_local, var1_index, var2_index, var1, var2);
				fprintf(ofp_task_created_c, "\tinsert_can_msg(&waiting_data, can_msg);\n");
			}
//			fprintf(ofp_task_created_c, "\t}\n");	// end if
		}
	
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
		
		// for task_instanciation.hh
		fprintf(ofp_task_instanciation_h, "case %d:\n", t_id);
		fprintf(ofp_task_instanciation_h, "\tjob = new Task%d(e->task_info_link);\n", t_id);
		fprintf(ofp_task_instanciation_h, "\tbreak;\n\n");
    }
    fprintf(ofp_task_created_h, "\n#endif\n");
    fclose(ofp_task_created_h);
    fclose(ofp_task_created_c);
    fclose(ofp_task_instanciation_h);
}

void DesignParser::getSource() {
	const char* filename = "design.xml";
	xmlDocPtr doc;
	xmlNodePtr cur;
	doc = xmlParseFile(filename);

	cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		if((!xmlStrcmp(cur->name, (const xmlChar*)"ECUs"))) {
			parseECUs(doc, cur);
		}
		cur = cur->next;
	}

	cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		if((!xmlStrcmp(cur->name, (const xmlChar*)"SWCs"))) {
			parseSWCs(doc, cur);
		}
		cur = cur->next;
	}

	generateTaskCode();

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
}

int main()
{
	DesignParser dp;
	dp.getSource();
	return 0;
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif
