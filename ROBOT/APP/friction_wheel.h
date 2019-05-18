#ifndef __FRICTION_WHEEL_H
#define __FRICTION_WHEEL_H

#include "bsp.h"


typedef struct
{
	u16 l_wheel_tarV;
	u16 r_wheel_tarV;

	u16 l_wheel_output;
	u16 r_wheel_output;	
	
	u16 l_wheel_output_slope;
	u16 r_wheel_output_slope;
	
}FRICTIONWHEEL_DATA;	//CAN1

#define FRICTIONWHEEL_DATA_DEFAULT \
{ \
	0,\
	0,\
	1000,\
	1000,\
	1000,\
	1000,\
}

void Friction_Speed_Set(u16 bullet_speed);	//�������٣����ٵ�Ħ�����ٶȵ�ת��

extern FRICTIONWHEEL_DATA frictionWheel_Data;

#endif
