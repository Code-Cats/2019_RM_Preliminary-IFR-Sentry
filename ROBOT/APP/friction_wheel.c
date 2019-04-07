#include "friction_wheel.h"

FRICTIONWHEEL_DATA frictionWheel_Data={0};

PID_GENERAL PID_Friction_Speed[2]={PID_FRICTION_SPEED_DEFAULT,PID_FRICTION_SPEED_DEFAULT};

extern u32 time_1ms_count;

void Friction_Task(void)	//3510Ä¦²ÁÂÖµ÷ËÙ¿ØÖÆ
{
	frictionWheel_Data.l_wheel_output=PID_General(frictionWheel_Data.l_wheel_tarV,frictionWheel_Data.l_wheel_fdbV,&PID_Friction_Speed[0]);
	frictionWheel_Data.r_wheel_output=PID_General(frictionWheel_Data.r_wheel_tarV,frictionWheel_Data.r_wheel_fdbV,&PID_Friction_Speed[1]);
}

