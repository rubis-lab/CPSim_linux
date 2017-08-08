var:0

input:0

output:1
internal_data[0]

CAN_input:1
0:0x7FD:1
car_output[SPEED]

CAN_output:0

code:
{
	internal_data[0] = car_output[SPEED];
}
