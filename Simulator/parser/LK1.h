var:0

input:0

output:2
internal_data[0]
internal_data[1]

CAN_input:1
0:0x7FC:2
car_output[TRACK_WIDTH]
car_output[TRACK_DISTANCE]

CAN_output:0

code:
{
	internal_data[0] = car_output[TRACK_WIDTH];
	internal_data[1] = car_output[DISTANCE];
}
