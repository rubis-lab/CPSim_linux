#ifndef __TASKCLASSH_
#define __TASKCLASSH_

#include "components.h"

// for data from car
#ifdef NOCANMODE
extern float *car_output;
#else
extern float car_output[10];
#endif
extern float user_input_internal[10];
extern float memory_buffer[10];

class Task0 : public Task		// LKAS
{
public:
	Task0();
	Task0(Task_info*);
	~Task0();

	virtual void read();
	virtual void procedure();
	virtual void write();
	float steering;
};

class Task1 : public Task		// dummy1
{
public:
	Task1();
	Task1(Task_info*);
	~Task1();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task2 : public Task		// dummy2
{
public:
	Task2();
	Task2(Task_info*);
	~Task2();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task3 : public Task		// CC1
{
public:
	Task3();
	Task3(Task_info*);
	~Task3();

	virtual void read();
	virtual void procedure();
	virtual void write();
	float speed;
};

class Task4 : public Task		// CC2
{
public:
	Task4();
	Task4(Task_info*);
	~Task4();

	virtual void read();
	virtual void procedure();
	virtual void write();
    float speed;
	float accel;
	float brake;
};

#endif
