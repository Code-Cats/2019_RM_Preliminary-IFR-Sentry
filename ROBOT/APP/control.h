#ifndef __CONTROL_H
#define __CONTROL_H

#include "sys.h"



float MyAbs(float num);
void Control_Task(void);

void Motor_Send(void);
void Work_Execute(void);	//工作执行2018.7.1
void Work_State_Change(void);
void Teleconltroller_Data_protect(void);	//遥控器数据自保护

/***********************--工作状态--**********************/
typedef enum
{
    PREPARE_STATE,    	//上电后初始化状态 1s钟左右
    CHECK_STATE,		//自检状态 在此阶段能够通过内核重启解决的问题发生时将会自动内核重启
	CALI_STATE,    		//校准状态标定
	LOST_STATE,			//硬件帧率丢失
    NORMAL_STATE,		//正常输入状态	手动状态
	AUTO_STATE,			//自动巡行状态
    STOP_STATE,      	//停止运动状态(由遥控器控制的保护状态)
	ERROR_STATE,		//硬件错误状态，包括但不限于检测出传感器失效，数据失常等重启无法解决的问题
	TEST_STATE,			//自定义的测试状态，开机到此状态则不参与错误检测等等
}WorkState_e;

extern WorkState_e workState;
void SetWorkState(WorkState_e state);
WorkState_e GetWorkState(void);

void Reset_Task(void);
void RC_Calibration(void);	//上电检测遥控器接收值并与默认参数比较，判断是否正常，否则软复位

#endif
