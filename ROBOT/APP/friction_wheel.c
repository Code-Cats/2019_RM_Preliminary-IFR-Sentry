#include "friction_wheel.h"

FRICTIONWHEEL_DATA frictionWheel_Data=FRICTIONWHEEL_DATA_DEFAULT;

s16 fireRateOffset=0;	//���ٶ�̬����
extern u32 time_1ms_count;
extern s16 bulletSpeedSet;
//5000 16��16.3��15.7��16.5	��7000 25 6000 21.08 20.97
//new:28-1200  7-1100 19-1150  15-1140  16-1145  19-1155  20.2-1160 22-1170
void Friction_Speed_Set(u16 bullet_speed)	//�������٣����ٵ�Ħ�����ٶȵ�ת��
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
		frictionWheel_Data.l_wheel_output=1115+fireRateOffset+(frictionWheel_Data.l_wheel_tarV-10)*5;	//1115->speed=10  1120 1105
	}
	
	
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

#define FIRERATEOFFSET_REDNUMS 150
s16 fireRateOffsetLast[FIRERATEOFFSET_REDNUMS];
u16 fireRateOffsetLastcount=0;
void Record_fireRateOffset(s16 offset)	//��¼fireRateOffset���ݵ���Ƶ��1ms
{
	fireRateOffsetLast[fireRateOffsetLastcount]=offset;
	fireRateOffsetLastcount++;
	if(fireRateOffsetLastcount>=FIRERATEOFFSET_REDNUMS)
	{
		fireRateOffsetLastcount=0;
	}
}

s16 GetRecordfireRateOffset(u16 lastcount)	//��ȡ��ȥ��fireRateOffset
{
	lastcount=lastcount>(FIRERATEOFFSET_REDNUMS-1)?(FIRERATEOFFSET_REDNUMS-1):lastcount;
	s16 lastindex=(s16)fireRateOffsetLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+FIRERATEOFFSET_REDNUMS):lastindex;
	return fireRateOffsetLast[lastindex];
}

///@breif �Զ��������ٲ��� �����ٲ��з����е���
void AutoAdjust_FrictionSpeed(float fdbv)
{
	if(ABS(fdbv-frictionWheel_Data.l_wheel_tarV)>1&&frictionWheel_Data.l_wheel_tarV!=0)
	{
		fireRateOffset=(s16)(GetRecordfireRateOffset(120)+(frictionWheel_Data.l_wheel_tarV-fdbv)*2);
		fireRateOffset=fireRateOffset>20?20:fireRateOffset;
		fireRateOffset=fireRateOffset<-20?-20:fireRateOffset;
	}
}


/**
@breif ���ݵ�ǰ���õ��ٺ���Ƶ����ĵ�ʵ�ʵ���
*/
u16 Get_Friction_Speed(u16 freque)
{
	  
}