var:4
float track_angle;
float track_width;
float distance;
float yaw;

input:0

output:4
track_angle
track_width
distance
yaw

CAN_input:2
0:0x7FC:2
car_output[TRACK_ANGLE]
car_output[TRACK_WIDTH]
4:0x7F5:2
car_output[DISTANCE]
car_output[YAW]

CAN_output:0

code:
{
	track_angle = car_output[TRACK_ANGLE];
	track_width = car_output[TRACK_WIDTH];
	distance = car_output[DISTANCE];
	yaw = car_output[YAW];
}
