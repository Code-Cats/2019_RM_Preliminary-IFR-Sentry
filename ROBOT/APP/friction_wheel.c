#include "friction_wheel.h"

FRICTIONWHEEL_DATA frictionWheel_Data={0};

PID_GENERAL PID_Friction_Speed[2]={PID_FRICTION_SPEED_DEFAULT,PID_FRICTION_SPEED_DEFAULT};

extern u32 time_1ms_count;
extern s16 bulletSpeedSet;
//5000 16，16.3，15.7，16.5	；7000 25 6000 21.08 20.97
void Friction_Speed_Set(s16 bullet_speed)	//设置射速，射速到摩擦轮速度的转换
{
	if(bullet_speed==16)
	{
		frictionWheel_Data.l_wheel_tarV=5000;
		frictionWheel_Data.r_wheel_tarV=-5000;
		bulletSpeedSet=16;
	}
	else if(bullet_speed==21)
	{
		frictionWheel_Data.l_wheel_tarV=6000;
		frictionWheel_Data.r_wheel_tarV=-6000;
		bulletSpeedSet=21;
	}
	else if(bullet_speed==25)
	{
		frictionWheel_Data.l_wheel_tarV=7000;
		frictionWheel_Data.r_wheel_tarV=-7000;
		bulletSpeedSet=25;
	}
	else if(bullet_speed==28)
	{
		frictionWheel_Data.l_wheel_tarV=8000;
		frictionWheel_Data.r_wheel_tarV=-8000;
		bulletSpeedSet=28;
	}
	else
	{
		frictionWheel_Data.l_wheel_tarV=0;
		frictionWheel_Data.r_wheel_tarV=-0;
		bulletSpeedSet=0;
	}
}

void Friction_Task(void)	//3510摩擦轮调速控制
{
	frictionWheel_Data.l_wheel_output=PID_General(frictionWheel_Data.l_wheel_tarV,frictionWheel_Data.l_wheel_fdbV,&PID_Friction_Speed[0]);
	frictionWheel_Data.r_wheel_output=PID_General(frictionWheel_Data.r_wheel_tarV,frictionWheel_Data.r_wheel_fdbV,&PID_Friction_Speed[1]);
}

