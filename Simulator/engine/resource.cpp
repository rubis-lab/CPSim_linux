#include "stdafx.h"
#include "components.h"

// constructor for Resource class
Resource::Resource(int id_input, int type_input, int ratio_input, int speed_input, char *name):Component(id_input)
{
	type = type_input;
	ratio = ratio_input;
	speed = speed_input;
	strcpy(resource_name, name);
}

Resource::~Resource()
{
}

// This function returns 'type' variable in Resource class
int Resource::get_type()
{
	return type;
}

// This function returns 'ratio' variable in Resource class
int Resource::get_ratio()
{
	return ratio;
}

// This function returns 'speed' variable in Resource class
int Resource::get_speed()
{
	return speed;
}

// This function returns 'resource_name' variable in Resource class
char* Resource::get_resource_name()
{
	return resource_name;
}
