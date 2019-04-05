#include "chassis.h"
#include "control.h"
#include "protect.h"

CHASSIS_DATA chassis_Data={0};

PID_GENERAL PID_Chassis_Speed[2]={PID_CHASSIS_SPEED_DEFAULT,PID_CHASSIS_SPEED_DEFAULT};
PID_GENERAL PID_Chassis_Follow=PID_CHASSIS_FOLLOW_DEFAULT;

s16 Chassis_Vx=0;


/************************外部数据引用**************************/
extern RC_Ctl_t RC_Ctl;	//遥控数据
extern u32 time_1ms_count;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*************************************************************/

/***************底盘相关控制位（其他函数会引用）*****************/

/*************************************************************/

#define K_SPEED 10

float t_Vx_k=0;
u8 Chassis_Control_RCorPC=RC_CONTROL;


void Remote_Task(void)
{
	Chassis_Control_External_Solution();
}



/******************************************************/
void Chassis_Control_External_Solution(void)	//陀螺仪正常的底盘解决方案
{
	static u8 chassis_follow_statu_last=0;	//记录上一次状态、目的是消除按下键盘触发不跟随模式和键盘控制模式有Vw残留的问题
	
	if(GetWorkState()==NORMAL_STATE)	//底盘PID复位
	{
		for(int i=0;i<2;i++)
		{
			PID_Chassis_Speed[i].k_i=CHASSIS_SPEED_PID_I;
			PID_Chassis_Speed[i].i_sum_max=CHASSIS_SPEED_I_MAX;
		}
	}

	
	if(GetWorkState()==NORMAL_STATE)
	{
		RC_Control_Chassis();
	}
	
				
	chassis_Data.lf_wheel_tarV=(Chassis_Vx)*K_SPEED;
	chassis_Data.rf_wheel_tarV=(-Chassis_Vx)*K_SPEED;

	
	chassis_Data.lf_wheel_output=PID_General(chassis_Data.lf_wheel_tarV,chassis_Data.lf_wheel_fdbV,&PID_Chassis_Speed[LF]);
	chassis_Data.rf_wheel_output=PID_General(chassis_Data.rf_wheel_tarV,chassis_Data.rf_wheel_fdbV,&PID_Chassis_Speed[RF]);

	
	
//	{	//功率限制块
//		float limit_k=Limit_Power(testPowerHeatData.chassisPower,testPowerHeatData.chassisPowerBuffer);	//testPowerHeatData.chassisPowerBuffer
//		float output_limit_lf=chassis_Data.lf_wheel_output*limit_k;
//		float output_limit_rf=chassis_Data.rf_wheel_output*limit_k;
//		float output_limit_lb=chassis_Data.lb_wheel_output*limit_k;
//		float output_limit_rb=chassis_Data.rb_wheel_output*limit_k;
//		chassis_Data.lf_wheel_output=(s32)output_limit_lf;
//		chassis_Data.rf_wheel_output=(s32)output_limit_rf;
//		chassis_Data.lb_wheel_output=(s32)output_limit_lb;
//		chassis_Data.rb_wheel_output=(s32)output_limit_rb;
//		
//	if(chassis_Data.lf_wheel_output>10000)	chassis_Data.lf_wheel_output=10000;
//	if(chassis_Data.lf_wheel_output<-10000)	chassis_Data.lf_wheel_output=-10000;
//	if(chassis_Data.rf_wheel_output>10000)	chassis_Data.rf_wheel_output=10000;
//	if(chassis_Data.rf_wheel_output<-10000)	chassis_Data.rf_wheel_output=-10000;
//	if(chassis_Data.lb_wheel_output>10000)	chassis_Data.lb_wheel_output=10000;
//	if(chassis_Data.lb_wheel_output<-10000)	chassis_Data.lb_wheel_output=-10000;
//	if(chassis_Data.rb_wheel_output>10000)	chassis_Data.rb_wheel_output=10000;
//	if(chassis_Data.rb_wheel_output<-10000)	chassis_Data.rb_wheel_output=-10000;
//	}
}
/******************************************************/





/******************************************************/



void RC_Control_Chassis(void)
{
	static s16 Chassis_Vx_last=0;
	if(GetWorkState()==NORMAL_STATE)	//如果在普通模式且补弹=0
	{
		if(time_1ms_count%1==0)
		{
			if(RC_Ctl.rc.ch0-1024-Chassis_Vx_last>1&&RC_Ctl.rc.ch0-1024>10)	//只在前进加速时生效，
			{
				Chassis_Vx+=1;
			}
			else if(RC_Ctl.rc.ch0-1024-Chassis_Vx_last<-1&&RC_Ctl.rc.ch0-1024<-10)	//只在后退加速时生效
			{
				Chassis_Vx-=1;
			}
			else
			{
				Chassis_Vx=RC_Ctl.rc.ch0-1024;
			}
		}
//		Chassis_Vx=RC_Ctl.rc.ch1-1024;	//代替为斜坡函数
		Chassis_Vx_last=Chassis_Vx;
	}
	
}



#define POWER_LIMIT_K 0.8f/50.0f	//即能量槽空时0.2，50时开始限制
#define POWER_LIMIT_B	0.21f
u8 limit_power_statu=0;
extern u8 SuperC_Output_Enable;	//电容是否能放电
extern Error_check_t Error_Check;
#define POWERLIMIT 120 	//120w功率限制
#define POWERBUFFER 60	//60J功率缓冲
float Limit_Power(float power,float powerbuffer)	//英雄120J热量限制，直接限制总输出
{
	float limit_k=1;
//	if(power>POWERLIMIT*0.6)
//	{
////		limit_k=3.0f*powerbuffer/200.0f+0.2f;	//0.4
////		limit_k=limit_k>1?1:limit_k;
////		limit_k=limit_k<0.1f?0.1f:limit_k;
//	}
//	limit_k=0.9;	//取消功率限制，屏蔽后取消
////////////	if(SuperC_Output_Enable==0)//电容不能放电
////////////	{
////////////		limit_power_statu=1;
////////////		limit_k=3.0f*powerbuffer/200.0f+0.08f;	//0.4
////////////		limit_k=limit_k>1?1:limit_k;
////////////		limit_k=limit_k<0.1f?0.1f:limit_k;
////////////	}
////////////	else	//电容可以放电，放宽限制
////////////	{
////////////		limit_power_statu=2;
////////////		limit_k=3.0f*powerbuffer/125.0f+0.1f;//+0.16f;//+0.25f;	//30j开始限制
////////////		limit_k=limit_k>1?1:limit_k;
////////////		limit_k=limit_k<0.1f?0.1f:limit_k;
////////////	}

	
	limit_k=POWER_LIMIT_K*powerbuffer+POWER_LIMIT_B;	//0.4
	limit_k=limit_k>1?1:limit_k;
	limit_k=limit_k<0.1f?0.1f:limit_k;
	
	if(Error_Check.statu[LOST_REFEREE]==1)	//裁判lost
	{
		limit_power_statu=3;
		limit_k=0.6;
	}
	
	return limit_k;
}

