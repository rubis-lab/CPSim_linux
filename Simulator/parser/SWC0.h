var:1
float speed;

input:0

output:1
speed

CAN_input:1
0:0x7FD:1
car_output[SPEED]

CAN_output:0

code:
{
	speed = car_output[SPEED];
}
