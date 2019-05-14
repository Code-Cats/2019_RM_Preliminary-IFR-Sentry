#include "control.h"
#include "yun.h"
#include "chassis.h"
#include "protect.h"
#include "friction_wheel.h"
#include "brain.h"
#include "heat_limit.h"
#include "led_control.h"

WorkState_e workState=PREPARE_STATE;

//uint32_t time_tick_1ms = 0;	//全局1ms计时变量
///***************************************************************************/
//WorkState_e workState = PREPARE_STATE;

//void SetWorkState(WorkState_e state)
//{
//    workState = state;
//}

//WorkState_e GetWorkState(void)
//{
//	return workState;
//}
/***************************************************************************/

extern s8 led_num;
extern YUN_MOTOR_DATA 			yunMotorData;
extern float ZGyroModuleAngle;
extern RC_Ctl_t RC_Ctl;
extern SHOOT_MOTOR_DATA shoot_Motor_Data_Down;


void Control_Task()
{
	if(time_1ms_count%10==0)
	{
		WS2812_Run();
	}
	
	//if(IMU_Read==1)
	//{
		MPU_get_Data();
	//}
	if(time_1ms_count%10==0)
	Heat_Simulating();
	
	Check_Task();
	
	Work_State_Change();
	Work_Execute();
	
	Motor_Send(); 
	
	Teleconltroller_Data_protect();
	
	if(time_1ms_count%120==0)
	{
		if(led_num>0)
		led_num--;
		Green_LED_8_SetNum(led_num);
	}
}



void Work_State_Change(void)
{
	static u8 Switch_Right_Last=0;
	static WorkState_e State_Record=CHECK_STATE;	
	State_Record=GetWorkState();
	
	
	switch (GetWorkState())	//2018.3.15
	{
		case CHECK_STATE:	//自检模式
		{	//板载外设初始化后便进入自检模式 
			
			break;
		}
		case PREPARE_STATE:	//预备模式
		{	
			
			break;
		}
		case CALI_STATE:	//标定模式
		{
			
			break;
		}
		case NORMAL_STATE:	//正常操作模式
		{
			if(RC_Ctl.rc.switch_left==RC_SWITCH_MIDDLE)	//左中右中		&&RC_Ctl.rc.switch_right==RC_SWITCH_MIDDLE
			{
				SetWorkState(STOP_STATE);
			}
			
			if(RC_Ctl.rc.switch_left==RC_SWITCH_MIDDLE&&Switch_Right_Last==RC_SWITCH_MIDDLE&&RC_Ctl.rc.switch_right==RC_SWITCH_UP)
			{
//				SetWorkState(ASCEND_STATE);
			}
			else if(RC_Ctl.rc.switch_left==RC_SWITCH_MIDDLE&&Switch_Right_Last==RC_SWITCH_MIDDLE&&RC_Ctl.rc.switch_right==RC_SWITCH_DOWN)
			{
//				SetWorkState(DESCEND_STATE);
//				SetWorkState(TAKEBULLET_STATE);	//增加新模式//临时测试，取弹状态
			}
			
			break;
		}
		case ERROR_STATE:	//错误模式
		{
			if(RC_Ctl.key.v_h!=0||RC_Ctl.key.v_l!=0||abs(RC_Ctl.mouse.x)>3)	//退出该模式
			{
				SetWorkState(NORMAL_STATE);
			}
			if(RC_Ctl.rc.switch_left==RC_SWITCH_DOWN)
			{
				SetWorkState(AUTO_STATE);
			}
			break;
		}
		case STOP_STATE:	//停止状态
		{
			if(RC_Ctl.rc.switch_left==RC_SWITCH_UP)	
			{
				SetWorkState(NORMAL_STATE);
			}
			else if(RC_Ctl.rc.switch_left==RC_SWITCH_DOWN)
			{
				SetWorkState(AUTO_STATE);
			}
			
			break;
		}
		case LOST_STATE:	//自我保护模式
		{
			static u32 time_count=0;
			time_count++;
			if(Error_Check.statu[LOST_DBUS]==0&&abs(RC_Ctl.rc.ch0+RC_Ctl.rc.ch1+RC_Ctl.rc.ch2+RC_Ctl.rc.ch3-1024*4)<10)
			{
				yunMotorData.yaw_tarP=(ZGyroModuleAngle*10+(YAW_INIT-yunMotorData.yaw_fdbP)*3600/8192);	//重置云台目标位置
				SetWorkState(NORMAL_STATE);
				time_count=0;
			}
			
			if(Error_Check.statu[LOST_DBUS]==0&&time_count>8000)	//有反馈认为无法恢复	（数据错乱）
			{
				time_count=0;
				NVIC_SystemReset();
			}
			break;
		}
		case AUTO_STATE:	//停止状态
		{
			if(RC_Ctl.rc.switch_left==RC_SWITCH_MIDDLE)	
			{
				SetWorkState(STOP_STATE);
			}
			break;
		}
	}
	Switch_Right_Last=RC_Ctl.rc.switch_right;
}


void Work_Execute(void)	//工作执行2018.7.1
{
	switch (GetWorkState())	//2018.3.15	执行任务块
	{
		case CHECK_STATE:	//自检模式
		{	//板载外设初始化后便进入自检模式 //此时外设刚刚开启，需等待一段时间全局自检未检测到异常（2-3个自检触发周期以上），又因为时间计算起点为定时器启动点，故无需进行时间差记录
			if(time_1ms_count>10000)	//若从LOST状态回到CHECK模式，则执行计数清零操作
			{	//若能执行到这里说明LOSTCHECK通过，进行数值检测
				RC_Calibration();	//self check
				if(1)	//selfcheck标志
				{
					SetWorkState(PREPARE_STATE);	//此步意味自检通过，一切硬件模块正常
				  	//数据初始化↓
					yunMotorData.pitch_tarP=PITCH_INIT;	//	//陀螺仪正方向云台向下
					yunMotorData.yaw_tarP=(ZGyroModuleAngle*10+(YAW_INIT-yunMotorData.yaw_fdbP)*3600/8192);	//反馈放大10倍并将目标位置置为中点
				}
			}
			break;
		}
		case PREPARE_STATE:	//预备模式
		{	//等待车身状态稳定，并设置初值
			Yun_Task();	//开启云台
//			if(abs(Gyro_Data.angvel[0])<20&&abs(Gyro_Data.angvel[2])<20&&abs(yunMotorData.pitch_tarP-(Gyro_Data.angle[0]*8192/360.0f+PITCH_INIT))<50)	//云台已就位	//位置环情况下
		//	if(abs(Gyro_Data.angvel[YAW])<2)	//云台已就位，且有反馈
			{
				SetWorkState(CALI_STATE);
			}
			Shoot_Task();	//临时调试
			break;
		}
		case CALI_STATE:	//标定模式
		{
			if(1)	//改为闭环标定	//BulletRotate_OffSetInit()==1		//BulletRotate_Cali()==1&& 临时取消夹子一系列功能，用于招新发传单 2018.9.9
			{
				SetWorkState(STOP_STATE);
			}
			Yun_Task();	//开启云台处理
			Shoot_Task();	//临时调试
			break;
		}
		case NORMAL_STATE:	//正常操作模式
		{
			Yun_Task();	//开启云台处理
			Shoot_Task();
			Friction_Task();
			Remote_Task();
			break;
		}
		case ERROR_STATE:	//错误模式
		{
			
			break;
		}
		case LOST_STATE:	//错误模式
		{
			break;
		}
		case STOP_STATE:	//停止状态
		{
			Remote_Task();
//////			Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
			Yun_Task();	//开启云台
		//	Vision_Task(&t_yaw_error,&t_pitch_error);
			break;
		}
		case AUTO_STATE:	//停止状态
		{
			Auto_Operation();	//自动运行
			
			Yun_Task();	//开启云台处理
			Shoot_Task();
			Friction_Task();
			Remote_Task();
			break;
		}
	}
}

extern u8 Friction_State;
extern CHASSIS_DATA chassis_Data;
void Motor_Send(void)
{
	switch (GetWorkState())	//2018.3.15
	{	
		case CHECK_STATE:	//自检模式
		{	//板载外设初始化后便进入自检模式 //此时外设刚刚开启，需等待一段时间全局自检未检测到异常（2-3个自检触发周期以上），又因为时间计算起点为定时器启动点，故无需进行时间差记录
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case PREPARE_STATE:	//预备模式
		{	//等待车身状态稳定，并设置初值
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case CALI_STATE:	//标定模式
		{
//			SetFrictionWheelSpeed(FRICTION_INIT);
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case NORMAL_STATE:	//正常操作模式
		{
			CAN1_Chassis_SendMsg(chassis_Data.lf_wheel_output,chassis_Data.rf_wheel_output,frictionWheel_Data.l_wheel_output,frictionWheel_Data.r_wheel_output);	//Friction_State*8000,Friction_State*-8000
			CAN1_Yun_Shoot_SendMsg(yunMotorData.yaw_output,yunMotorData.pitch_output,shoot_Motor_Data_Down.output,0);
			//CAN1_Yun_Shoot_SendMsg(yunMotorData.yaw_output,0,0,0);	//yaw pitch
			break;
		}
		case ERROR_STATE:	//错误模式
		{
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case STOP_STATE:	//停止状态
		{
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case AUTO_STATE:	//停止状态
		{
			CAN1_Chassis_SendMsg(chassis_Data.lf_wheel_output,chassis_Data.rf_wheel_output,frictionWheel_Data.l_wheel_output,frictionWheel_Data.r_wheel_output);
			CAN1_Yun_Shoot_SendMsg(yunMotorData.yaw_output,yunMotorData.pitch_output,shoot_Motor_Data_Down.output,0);
			break;
		}
		default:
		{
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
	}
}
	


void RC_Calibration(void)	//上电检测遥控器接收值并与默认参数比较，判断是否正常，否则软复位
{													//注：必须放在遥控器接收初始化后
	if(abs(RC_Ctl.rc.ch0+RC_Ctl.rc.ch1+RC_Ctl.rc.ch2+RC_Ctl.rc.ch3-1024*4)>8)
	{
		NVIC_SystemReset();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************主动保护状态***********************************************/
RC_Ctl_t RC_DATA_ERROR={0};	//记录错误帧数据
void Teleconltroller_Data_protect(void)	//遥控器数据自保护 
{
	u8 protect_state=0xC0;	//按位表示当前遥控器数据是否正常	//最高2位为保留位，常为1	//364-1024-1684
	protect_state|=(abs(RC_Ctl.rc.ch0-1024)<=662);
	protect_state|=(abs(RC_Ctl.rc.ch1-1024)<=662)<<1;
	protect_state|=(abs(RC_Ctl.rc.ch2-1024)<=662)<<2;
	protect_state|=(abs(RC_Ctl.rc.ch3-1024)<=662)<<3;
	protect_state|=(RC_Ctl.rc.switch_left==1||RC_Ctl.rc.switch_left==2||RC_Ctl.rc.switch_left==3)<<4;
	protect_state|=(RC_Ctl.rc.switch_right==1||RC_Ctl.rc.switch_right==2||RC_Ctl.rc.switch_right==3)<<5;
	
	if(protect_state!=0xFF)	{SetWorkState(LOST_STATE); RC_DATA_ERROR=RC_Ctl;}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////



//求绝对值函数
float MyAbs(float num)
{
	if(num>=0)
		return num;
	else 
		return -num;	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/***********************--工作状态--**********************/
void SetWorkState(WorkState_e state)
{
    workState = state;
}


WorkState_e GetWorkState()
{
	return workState;
}
