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
    if(speed < 22.0) // 50?
        accel = 3.5;
    else if(speed < 25.0) // 60
        accel = 0.5;
    else if(speed < 30.0) // 80
        accel = 0.2;
    else
        accel = brake;

//    if(speed < 50.0) // 50?
//        accel = 1.5;
//    else if(speed < 60.0) // 60
//        accel = 0.5;
//    else if(speed < 80.0) // 80
//        accel = 0.2;
//    else
//        accel = brake;
}
