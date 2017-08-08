var:2
float accel;
float brake;

input:0

output:0

CAN_input:0

CAN_output:1
10:0x7FF:2
accel ACCEL
brake BRAKE

code:
{
	float speed = internal_data[0];
	brake = 0.0;

    if(speed < 75.0)
        accel = 1;
    else if(speed < 100.0)
        accel = 0.2;
    else if(speed < 140.0)
        accel = 0.1;
    else
        accel = 0.0;
}
