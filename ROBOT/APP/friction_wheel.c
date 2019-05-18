#include "friction_wheel.h"

FRICTIONWHEEL_DATA frictionWheel_Data=FRICTIONWHEEL_DATA_DEFAULT;

extern u32 time_1ms_count;
extern s16 bulletSpeedSet;
//5000 16��16.3��15.7��16.5	��7000 25 6000 21.08 20.97
void Friction_Speed_Set(u16 bullet_speed)	//�������٣����ٵ�Ħ�����ٶȵ�ת��
{
	if(bullet_speed==16)
	{
		frictionWheel_Data.l_wheel_tarV=16;
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
	
	frictionWheel_Data.l_wheel_output=1000+frictionWheel_Data.l_wheel_tarV*10+5;
	
	//б�º���
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
@breif ���ݵ�ǰ���õ��ٺ���Ƶ����ĵ�ʵ�ʵ���
*/
u16 Get_Friction_Speed(u16 freque)
{
	
}