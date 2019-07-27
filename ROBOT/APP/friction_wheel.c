#include "friction_wheel.h"

FRICTIONWHEEL_DATA frictionWheel_Data=FRICTIONWHEEL_DATA_DEFAULT;

extern u32 time_1ms_count;
extern s16 bulletSpeedSet;
//5000 16，16.3，15.7，16.5	；7000 25 6000 21.08 20.97
//new:28-1200  7-1100 19-1150  15-1140  16-1145  19-1155  20.2-1160 22-1170
void Friction_Speed_Set(u16 bullet_speed)	//设置射速，射速到摩擦轮速度的转换
{
	if(bullet_speed==16)
	{
		frictionWheel_Data.l_wheel_tarV=16;
	}
	else if(bullet_speed==18)
	{
		frictionWheel_Data.l_wheel_tarV=18;
	}
	else if(bullet_speed==21)
	{
		frictionWheel_Data.l_wheel_tarV=21;
	}
	else if(bullet_speed==25)
	{
		frictionWheel_Data.l_wheel_tarV=25;
	}
	else if(bullet_speed==28)
	{
		frictionWheel_Data.l_wheel_tarV=28;
	}
	else
	{
		frictionWheel_Data.l_wheel_tarV=0;
	}
	
	bulletSpeedSet=frictionWheel_Data.l_wheel_tarV;	//test
	
	if(frictionWheel_Data.l_wheel_tarV<12)
	{
		frictionWheel_Data.l_wheel_output=1000;
	}
	else
	{
		frictionWheel_Data.l_wheel_output=1118+(frictionWheel_Data.l_wheel_tarV-10)*5;	//1115->speed=10  1120
	}
	
	
	//斜坡函数
	if(frictionWheel_Data.l_wheel_output_slope-frictionWheel_Data.l_wheel_output>=1)
	{
		frictionWheel_Data.l_wheel_output_slope--;
	}
	else if(frictionWheel_Data.l_wheel_output_slope-frictionWheel_Data.l_wheel_output<=-1)
	{
		frictionWheel_Data.l_wheel_output_slope++;
	}
	//set pwm
	SetFrictionWheelSpeed(frictionWheel_Data.l_wheel_output_slope);
}

/**
@breif 根据当前设置弹速和射频推理的到实际弹速
*/
u16 Get_Friction_Speed(u16 freque)
{
	
}