#include "components.h"

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

int Resource::get_type()
{
	return type;
}

int Resource::get_ratio()
{
	return ratio;
}

int Resource::get_speed()
{
	return speed;
}

char* Resource::get_resource_name()
{
	return resource_name;
}
