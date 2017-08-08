var:3
float speed;
float accel;
float brake;

input:1
speed

output:0

CAN_input:0

CAN_output:1
10:0x7FF:2
accel ACCEL
brake BRAKE

code:
{
    brake = 0.0;
    if(speed < 10.0)
        accel = 1.0;
    else if(speed < 60.0)
        accel = 0.5;
    else if(speed < 80.0)
        accel = 0.2;
    else
        accel = 0.0;
}
