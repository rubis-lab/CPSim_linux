		case 0x7F5:
			memcpy(&car_output[LATERAL_DISTANCE], &(msg.DATA[0]), sizeof(float));
			break;

		case 0x7FB:
			memcpy(&car_output[RPM], &(msg.DATA[0]), sizeof(float));
			memcpy(&car_output[YAW_RATE], &(msg.DATA[4]), sizeof(float));
			break;

		case 0x7FD:
			memcpy(&car_output[SPEED], &(msg.DATA[0]), sizeof(float));
			break;
