var:0

input:0

output:2
internal_data[2]
internal_data[3]

CAN_input:1
4:0x7F5:2
car_output[TRACK_ANGLE]
car_output[YAW]

CAN_output:0

code:
{
	internal_data[2] = car_output[TRACK_ANGLE];
	internal_data[3] = car_output[YAW];
}
