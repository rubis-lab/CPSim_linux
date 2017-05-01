#ifndef __TASKCLASSH_
#define __TASKCLASSH_

#include "components.h"

// for data from car
extern float car_output[1000];

extern float user_input[10];
extern float memory_buffer[10];

class Task1 : public Task		// CC1
{
private:
	int data_size;
	float data[10];
	float speed;
	float accel;

public:
	Task1();
	~Task1();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task2 : public Task		// CC2
{
private:
	int data_size;
	float data[10];
	float accel;

public:
	Task2();
	~Task2();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task3 : public Task		// LK1
{
private:
	int data_size;
	float data[10];
	float lateral_distance;

public:
	Task3();
	~Task3();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task4 : public Task		// LK2
{
private:
	int data_size;
	float data[10];
	float lateral_distance;
	float steering;
	float lead_distance;

public:
	Task4();
	~Task4();

	virtual void read();
	virtual void procedure();
	virtual void write();
};

class Task5 : public Task		// LK3
{
private:
	int data_size;
	float data[10];
	float steering;
	float lead_distance;

public:
	Task5();
	~Task5();

	virtual void read();
	virtual void procedure();
	virtual void write();
};


#endif
