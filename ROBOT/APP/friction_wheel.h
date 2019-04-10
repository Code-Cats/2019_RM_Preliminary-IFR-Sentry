#ifndef __FRICTION_WHEEL_H
#define __FRICTION_WHEEL_H

#include "bsp.h"


#define FRICTION_SPEED_PID_P 3.0f	//底盘PID参数
#define FRICTION_SPEED_PID_I 0.02f//0.035f
#define FRICTION_SPEED_PID_D 0
#define FRICTION_SPEED_MAXOUT 9000
#define FRICTION_SPEED_MAXINPUT 9000
#define FRICTION_SPEED_MER 3000//3200
#define FRICTION_SPEED_DEAD 0
#define FRICTION_SPEED_I_MAX 1800/FRICTION_SPEED_PID_I


//#define LF 0
//#define RF 1
//#define LB 2
//#define RB 3

//底盘电机速度环PID参数
#define PID_FRICTION_SPEED_DEFAULT \
{\
	FRICTION_SPEED_PID_P,\
	FRICTION_SPEED_PID_I,\
  FRICTION_SPEED_PID_D,\
	FRICTION_SPEED_MER,\
	-FRICTION_SPEED_MAXINPUT,\
	FRICTION_SPEED_MAXINPUT,\
	-FRICTION_SPEED_MAXOUT,\
	FRICTION_SPEED_MAXOUT,\
	FRICTION_SPEED_I_MAX,\
	{0.0,0.0},\
	0.0,\
	0.0,\
	0.0,\
	FRICTION_SPEED_DEAD,\
}\




typedef struct
{
	int32_t l_wheel_fdbV; //201 
	int32_t r_wheel_fdbV; //202  

	int32_t l_wheel_tarV;
	int32_t r_wheel_tarV;

	int32_t l_wheel_fdbP;
	int32_t r_wheel_fdbP;

	int32_t l_wheel_tarP;
	int32_t r_wheel_tarP;

	int32_t l_wheel_output;
	int32_t r_wheel_output;	
	
}FRICTIONWHEEL_DATA;	//CAN1

void Friction_Speed_Set(s16 bullet_speed);	//设置射速，射速到摩擦轮速度的转换
void Friction_Task(void);	//3510摩擦轮调速控制

extern FRICTIONWHEEL_DATA frictionWheel_Data;

#endif
