case 0x7FD:
	memcpy(&car_output[SPEED], &(msg.DATA[0]), sizeof(float));
	break;

case 0x7FC:
	memcpy(&car_output[TRACK_WIDTH], &(msg.DATA[0]), sizeof(float));
	memcpy(&car_output[TRACK_DISTANCE], &(msg.DATA[4]), sizeof(float));
	break;

case 0x7F5:
	memcpy(&car_output[TRACK_ANGLE], &(msg.DATA[0]), sizeof(float));
	memcpy(&car_output[YAW], &(msg.DATA[4]), sizeof(float));
	break;

