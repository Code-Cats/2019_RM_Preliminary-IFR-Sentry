#include "chassis.h"
#include "control.h"
#include "protect.h"
#include "usart3_judge_analysis.h"

#include "auto_move.h"
#include "brain.h"

CHASSIS_DATA chassis_Data={0};

PID_GENERAL PID_Chassis_Speed[2]={PID_CHASSIS_SPEED_DEFAULT,PID_CHASSIS_SPEED_DEFAULT};
PID_GENERAL PID_Chassis_Follow=PID_CHASSIS_FOLLOW_DEFAULT;

s16 Chassis_Vx=0;


/************************�ⲿ��������**************************/
extern RC_Ctl_t RC_Ctl;	//ң������
extern u32 time_1ms_count;
extern ext_power_heat_data_t heat_data_judge;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*************************************************************/

/***************������ؿ���λ���������������ã�*****************/
extern AutoOperationDataTypeDef AutoOperationData;
/*************************************************************/

#define K_SPEED 15

float t_Vx_k=0;
u8 Chassis_Control_RCorPC=RC_CONTROL;

void Remote_Task(void)
{
	Chassis_Control_External_Solution();
}


float chassis_limit_k=0;

float now_wheel_output=0;
/******************************************************/
void Chassis_Control_External_Solution(void)	//�����������ĵ��̽������
{
	static u8 chassis_follow_statu_last=0;	//��¼��һ��״̬��Ŀ�����������¼��̴���������ģʽ�ͼ��̿���ģʽ��Vw����������
	
	if(GetWorkState()==NORMAL_STATE)	//����PID��λ
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
	
//	if(RC_Ctl.rc.switch_left==RC_SWITCH_DOWN)	//Ų��control.c��
//	{
//		Auto_Move_Task(0,0);
//	}
				
	Chassis_Vx=Chassis_Vx>1800?1800:Chassis_Vx;
	Chassis_Vx=Chassis_Vx<-1800?-1800:Chassis_Vx;
	
	chassis_Data.lf_wheel_tarV=(Chassis_Vx)*K_SPEED;
	chassis_Data.rf_wheel_tarV=(-Chassis_Vx)*K_SPEED;	///////////////////////////////////ͬ���������

	
	chassis_Data.lf_wheel_output=PID_General(chassis_Data.lf_wheel_tarV,chassis_Data.lf_wheel_fdbV,&PID_Chassis_Speed[LF]);
	now_wheel_output=PID_General(chassis_Data.rf_wheel_tarV,chassis_Data.rf_wheel_fdbV,&PID_Chassis_Speed[RF]);

	chassis_Data.rf_wheel_output=now_wheel_output;//0.5f*chassis_Data.rf_wheel_output+0.5f*
	
	if(GetWorkState()==AUTO_STATE&&AutoOperationData.chassis_enable==0)
	{
		chassis_Data.lf_wheel_output=0;
		chassis_Data.rf_wheel_output=0;
	}
		
	{	//�������ƿ�
		u32 outputsum=ABS(chassis_Data.lf_wheel_output)+ABS(chassis_Data.rf_wheel_output);
		chassis_limit_k=Limit_Power(heat_data_judge.chassis_power,heat_data_judge.chassis_power_buffer,outputsum);	//testPowerHeatData.chassisPowerBuffer
		float output_limit_lf=chassis_Data.lf_wheel_output*chassis_limit_k;
		float output_limit_rf=chassis_Data.rf_wheel_output*chassis_limit_k;
//		float output_limit_lb=chassis_Data.lb_wheel_output*limit_k;
//		float output_limit_rb=chassis_Data.rb_wheel_output*limit_k;
		chassis_Data.lf_wheel_output=(s32)output_limit_lf;
		chassis_Data.rf_wheel_output=(s32)output_limit_rf;
//		chassis_Data.lb_wheel_output=(s32)output_limit_lb;
//		chassis_Data.rb_wheel_output=(s32)output_limit_rb;

		if(chassis_Data.lf_wheel_output>10000)	chassis_Data.lf_wheel_output=10000;
		if(chassis_Data.lf_wheel_output<-10000)	chassis_Data.lf_wheel_output=-10000;
		if(chassis_Data.rf_wheel_output>10000)	chassis_Data.rf_wheel_output=10000;
		if(chassis_Data.rf_wheel_output<-10000)	chassis_Data.rf_wheel_output=-10000;
//		if(chassis_Data.lb_wheel_output>10000)	chassis_Data.lb_wheel_output=10000;
//		if(chassis_Data.lb_wheel_output<-10000)	chassis_Data.lb_wheel_output=-10000;
//		if(chassis_Data.rb_wheel_output>10000)	chassis_Data.rb_wheel_output=10000;
//		if(chassis_Data.rb_wheel_output<-10000)	chassis_Data.rb_wheel_output=-10000;
	}
}
/******************************************************/





/******************************************************/



void RC_Control_Chassis(void)
{
	static s16 Chassis_Vx_last=0;
	if(GetWorkState()==NORMAL_STATE)	//�������ͨģʽ�Ҳ���=0
	{
		if(time_1ms_count%1==0)
		{
			if(RC_Ctl.rc.ch0-1024-Chassis_Vx_last>1&&RC_Ctl.rc.ch0-1024>10)	//ֻ��ǰ������ʱ��Ч��
			{
				Chassis_Vx+=1;
			}
			else if(RC_Ctl.rc.ch0-1024-Chassis_Vx_last<-1&&RC_Ctl.rc.ch0-1024<-10)	//ֻ�ں��˼���ʱ��Ч
			{
				Chassis_Vx-=1;
			}
			else
			{
				Chassis_Vx=RC_Ctl.rc.ch0-1024;
			}
		}
//		Chassis_Vx=RC_Ctl.rc.ch1-1024;	//����Ϊб�º���
		Chassis_Vx_last=Chassis_Vx;
	}
	
}

extern ext_game_robot_state_t robot_state_judge;
//u8 Tar_remain_powerbuffer=160;
//����Ϊ40j��������
#define POWER_LIMIT_K 0.96f/42 //0.8f/50.0f	//�������ۿ�ʱ0.2��50ʱ��ʼ����
#define POWER_LIMIT_B	0.04f //0.21f
u8 limit_power_statu=0;
extern u8 SuperC_Output_Enable;	//�����Ƿ��ܷŵ�
extern Error_check_t Error_Check;
#define POWERLIMIT 20 	//20w��������
#define POWERBUFFER 200	//200J���ʻ���
float Limit_Power(float power,float powerbuffer,u32 outputsum)	//������ output=500����=20w ����400
{
	float limit_k=1;

	if(outputsum>500||AutoOperationData.real_remainbuffer<=50||powerbuffer<=50)//��Ҫ���²���
	{
		if(Error_Check.statu[LOST_REFEREE]==1)	//����lost
		{
			limit_power_statu=3;
			limit_k=0.5;
		}
		else
		{
			//powerbuffer-=AutoOperationData.real_remainbuffer;

			limit_k=POWER_LIMIT_K*powerbuffer+POWER_LIMIT_B;	//0.4
			limit_k=limit_k>1?1:limit_k;
			limit_k=limit_k<0.04f?0.04f:limit_k;
		}
	}


	
	return limit_k;
}

