#ifndef __MYDEF_H
#define __MYDEF_H

#include "sys.h"

#define ABS(x)	( (x>0) ? (x) : (-x) )

#define OFF 0
#define ON 1


#define WIFI_DEBUG_ENABLE 0
///********LED******/
//#define LIGHT 0

///********************************运动控制******************************/
//#define K_CM_USUAL 10	//底盘拨杆增益系数-普通模式
//#define K_PITCH 0.4f	//俯仰绝对位置拨杆系数
//#define K_YAW 0.005f	//水平云台增量拨杆系数


//#define SPEED_MAXDIFF 10	//task500HZ运算频率，最大速度6000-7000，使用斜坡使一秒最多变化5000





/***************************FRICTION**********************************/
//#define ESC_CYCLE 14000			//2.5ms周期

///***********取弹摩擦轮************/
//#define AM_FRICTION_OPEN 1500	//待试最优参数
//#define AM_FRICTION_CLOSE 1100  
///***********射击摩擦轮************/
//#define LU_FRICTION_OPEN 1600	//待试最优参数
//#define LU_FRICTION_CLOSE 1100  

/**********************************************************************/












/*
WorkState_e workState = PREPARE_STATE;

static void SetWorkState(WorkState_e state)
{
    workState = state;
}


WorkState_e GetWorkState()
{
	return workState;
}

//使用示例
if(GetWorkState()==PREPARE_STATE) //启动阶段，底盘不旋转
	{
		ChassisSpeedRef.rotate_ref = 0;	 
	}
*/	




#endif
