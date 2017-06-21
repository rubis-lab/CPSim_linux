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
	    angle -= 2*pi;
	while(angle < -pi)
	    angle += 2*pi;

		// 6.28
	/********************************************/
	float SC = 1.0;
	float abs_distance = distance;
	float left_distance = track_width/2 - distance;
	int tmp;
	float tail;
	if (distance < 0) {
		abs_distance = -distance;
	}
	if(track_width == 0.0) {
		steering = 0.0;
	}else {
		if (distance * angle > 0) {
			/* angle to center */
			if (distance < 0.7 && distance > -0.7) {
				/* close to the center */
				if (distance < 0) {
					/* left turning optimization */
					steering = -0.2 * angle - 0.8*distance/track_width;
				} else {
					steering = 1.9 * angle - 0.6*distance/track_width;
				}
				/* debug */
				tmp = int(steering * 1000000);
				tmp = tmp % 1000;
				tail = float(tmp) / 1000000;
				steering = steering > 0 ? steering - tail + 0.000050 : steering - tail - 0.000055;
			} else {
				/* far away center */
				steering = 1.7 * angle - 2.6*distance/track_width;
				/* debug */
				tmp = int(steering * 1000000);
				tmp = tmp % 1000;
				tail = float(tmp) / 1000000;
				steering = steering > 0 ? steering - tail + 0.000010 : steering - tail - 0.000015;
			}
		} else {
			float d_offset = 0.39;
			/* angle reverse to center */
			if (distance < 0.16 && distance > -0.16) {
				/* use linear offset */
				if (distance < 0) {
					d_offset = 2.5 * (-distance);
				} else {
					d_offset = 2.5 * (distance);
				}
			}
			/* set steering */
			if (angle > 0 && distance < 0) {// 2.3 3.8
				steering = 2.3 * angle - 4.05 * (distance - d_offset)/track_width;
			} else  {
				steering = 2.3 * angle - 3.99 * (distance + d_offset)/track_width;
			}
			/* debug */
			tmp = int(steering * 1000000);
			tmp = tmp % 1000;
			tail = float(tmp) / 1000000;
			steering = steering > 0 ? steering - tail + 0.000020 : steering - tail - 0.000025;
		}

//		steering = 3.23232323;



	}


}
