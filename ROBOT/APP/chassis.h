#ifndef __CHASSIS_H__
#define __CHASSIS_H__

#include "bsp.h"

#define RC_CONTROL 0	//遥控器操纵模式
#define PC_CONTROL 1	//键盘操纵模式

#define CHASSIS_SPEED_PID_P 3.0f	//底盘PID参数
#define CHASSIS_SPEED_PID_I 0.01f//0.035f
#define CHASSIS_SPEED_PID_D 0
#define CHASSIS_SPEED_MAXOUT 5000//9000
#define CHASSIS_SPEED_MAXINPUT 8000
#define CHASSIS_SPEED_MER 3000//3200
#define CHASSIS_SPEED_DEAD 0
#define CHASSIS_SPEED_I_MAX 1500/CHASSIS_SPEED_PID_I


#define LF 0
#define RF 1
//#define LB 2
//#define RB 3

//底盘电机速度环PID参数
#define PID_CHASSIS_SPEED_DEFAULT \
{\
	CHASSIS_SPEED_PID_P,\
	CHASSIS_SPEED_PID_I,\
  CHASSIS_SPEED_PID_D,\
	CHASSIS_SPEED_MER,\
	-CHASSIS_SPEED_MAXINPUT,\
	CHASSIS_SPEED_MAXINPUT,\
	-CHASSIS_SPEED_MAXOUT,\
	CHASSIS_SPEED_MAXOUT,\
	CHASSIS_SPEED_I_MAX,\
	{0.0,0.0},\
	0.0,\
	0.0,\
	0.0,\
	CHASSIS_SPEED_DEAD,\
}\

#define CHASSIS_FOLLOW_PID_P 0.85f//0.88f//0.57f//0.6f//0.6有一定超调2018.4.23
#define CHASSIS_FOLLOW_PID_I 0
#define CHASSIS_FOLLOW_PID_D 25//17
#define CHASSIS_FOLLOW_PID_MAXINPUT 7000//6000	//云台活动范围：4450-5350
#define CHASSIS_FOLLOW_PID_MAXOUT 550//400
#define CHASSIS_FOLLOW_PID_MER 700//600
#define CHASSIS_FOLLOW_PID_DEAD 6//8//10
#define CHASSIS_FOLLOW_PID_I_MAX 0

//底盘跟随速度环PID参数
#define PID_CHASSIS_FOLLOW_DEFAULT \
{\
	CHASSIS_FOLLOW_PID_P,\
	CHASSIS_FOLLOW_PID_I,\
  CHASSIS_FOLLOW_PID_D,\
	CHASSIS_FOLLOW_PID_MER,\
	-CHASSIS_FOLLOW_PID_MAXINPUT,\
	CHASSIS_FOLLOW_PID_MAXINPUT,\
	-CHASSIS_FOLLOW_PID_MAXOUT,\
	CHASSIS_FOLLOW_PID_MAXOUT,\
	CHASSIS_FOLLOW_PID_I_MAX,\
	{0.0,0.0},\
	0.0,\
	0.0,\
	0.0,\
	CHASSIS_FOLLOW_PID_DEAD,\
}\


typedef struct
{
	 int32_t lf_wheel_fdbV; //201 
	 int32_t rf_wheel_fdbV; //202  
	
   int32_t lf_wheel_tarV;
	 int32_t rf_wheel_tarV;
	  
   int32_t lf_wheel_fdbP;
	 int32_t rf_wheel_fdbP;
	 
	 int32_t lf_wheel_tarP;
	 int32_t rf_wheel_tarP;
	 
	 int32_t lf_wheel_output;
	 int32_t rf_wheel_output;	
	
}CHASSIS_DATA;	//CAN1


void Remote_Task(void);

void Chassis_Control_External_Solution(void);	//外置反馈方案

void RC_Control_Chassis(void);

float Limit_Power(float power,float powerbuffer,u32 outputsum); //同时根据输出来限制

#endif

