#ifndef __TASKCLASSH_
#define __TASKCLASSH_

#include "components.h"

// for data from car
#ifdef NOCANMODE
extern float *car_output;
#else
extern float car_output[10];
#endif
extern float memory_buffer[10];

class Task0 : public Task		// CC1
{
private:

public:
	Task0();
	Task0(Task_info*);
	~Task0();

	virtual void procedure();
	virtual void write();
};

class Task1 : public Task		// LK1
{
private:

public:
	Task1();
	Task1(Task_info*);
	~Task1();

	virtual void procedure();
	virtual void write();
};

class Task2 : public Task		// CC2
{
private:
	float accel;
	float brake;

public:
	Task2();
	Task2(Task_info*);
	~Task2();

	virtual void procedure();
	virtual void write();
};

class Task3 : public Task		// LK2
{
private:

public:
	Task3();
	Task3(Task_info*);
	~Task3();

	virtual void procedure();
	virtual void write();
};

class Task4 : public Task		// other1
{
private:

public:
	Task4();
	Task4(Task_info*);
	~Task4();

	virtual void procedure();
	virtual void write();
};

class Task5 : public Task		// other2
{
private:

public:
	Task5();
	Task5(Task_info*);
	~Task5();

	virtual void procedure();
	virtual void write();
};

class Task6 : public Task		// other3
{
private:

public:
	Task6();
	Task6(Task_info*);
	~Task6();

	virtual void procedure();
	virtual void write();
};

class Task7 : public Task		// LK3
{
private:
	float steering;

public:
	Task7();
	Task7(Task_info*);
	~Task7();

	virtual void procedure();
	virtual void write();
};


#endif
