#include "components.h"

Component::Component()
{
}

// constructor for Component class
Component::Component(int input)
{
	id = input;
}

Component::~Component()
{
}

// This function returns 'id' variable in Component class
int Component::get_id()
{
	return id;
}

// This function sets 'id' variable in Component class as 'input'
void Component::set_id(int input)
{
	id = input;
}
