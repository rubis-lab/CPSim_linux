var:1
float steering;

input:0

output:0

CAN_input:0

CAN_output:1
10:0x7FE:1
steering STEER

code:
{
	// sensor data
	float track_width = internal_data[0];
	float distance = internal_data[1];
	float track_angle = internal_data[2];
	float yaw = internal_data[3];

	// get difference of angle
	float angle = track_angle - yaw;
	float pi = 3.141592;
	while(angle > pi)
	    angle -= 2 * pi;
	while(angle < -pi)
	    angle += 2 * pi;

	// calculate steering value
	float SC = 1.4;
	if(track_width == 0.0) {	// for default value
		steering = 0.0;
	}
	else if (track_width < 2) {
		SC = 1;
		steering = angle - SC*distance/track_width;
	}
	else {
		steering = angle - SC*distance/track_width;
	}
}
