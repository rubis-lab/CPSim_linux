var:5
float track_angle;
float track_width;
float distance;
float yaw;
float angle;

input:4
track_angle
track_width
distance
yaw

output:0

CAN_input:0

CAN_output:1
10:0x7FE:1
angle STEER

code:
{
	angle = track_angle - yaw;
	float pi = 3.141592;
	while(angle > pi)
	    angle -= 2*pi;
	while(angle < -pi)
	    angle += 2*pi;
	float SC = 1.0;
	angle -= SC*distance/track_width;	// Road Keeping Assist System
}
