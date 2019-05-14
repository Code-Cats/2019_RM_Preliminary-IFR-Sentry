#include "control.h"
#include "yun.h"
#include "chassis.h"
#include "protect.h"
#include "friction_wheel.h"
#include "brain.h"
#include "heat_limit.h"
#include "led_control.h"

WorkState_e workState=PREPARE_STATE;

//uint32_t time_tick_1ms = 0;	//ȫ��1ms��ʱ����
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
		case CHECK_STATE:	//�Լ�ģʽ
		{	//���������ʼ���������Լ�ģʽ 
			
			break;
		}
		case PREPARE_STATE:	//Ԥ��ģʽ
		{	
			
			break;
		}
		case CALI_STATE:	//�궨ģʽ
		{
			
			break;
		}
		case NORMAL_STATE:	//��������ģʽ
		{
			if(RC_Ctl.rc.switch_left==RC_SWITCH_MIDDLE)	//��������		&&RC_Ctl.rc.switch_right==RC_SWITCH_MIDDLE
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
//				SetWorkState(TAKEBULLET_STATE);	//������ģʽ//��ʱ���ԣ�ȡ��״̬
			}
			
			break;
		}
		case ERROR_STATE:	//����ģʽ
		{
			if(RC_Ctl.key.v_h!=0||RC_Ctl.key.v_l!=0||abs(RC_Ctl.mouse.x)>3)	//�˳���ģʽ
			{
				SetWorkState(NORMAL_STATE);
			}
			if(RC_Ctl.rc.switch_left==RC_SWITCH_DOWN)
			{
				SetWorkState(AUTO_STATE);
			}
			break;
		}
		case STOP_STATE:	//ֹͣ״̬
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
		case LOST_STATE:	//���ұ���ģʽ
		{
			static u32 time_count=0;
			time_count++;
			if(Error_Check.statu[LOST_DBUS]==0&&abs(RC_Ctl.rc.ch0+RC_Ctl.rc.ch1+RC_Ctl.rc.ch2+RC_Ctl.rc.ch3-1024*4)<10)
			{
				yunMotorData.yaw_tarP=(ZGyroModuleAngle*10+(YAW_INIT-yunMotorData.yaw_fdbP)*3600/8192);	//������̨Ŀ��λ��
				SetWorkState(NORMAL_STATE);
				time_count=0;
			}
			
			if(Error_Check.statu[LOST_DBUS]==0&&time_count>8000)	//�з�����Ϊ�޷��ָ�	�����ݴ��ң�
			{
				time_count=0;
				NVIC_SystemReset();
			}
			break;
		}
		case AUTO_STATE:	//ֹͣ״̬
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


void Work_Execute(void)	//����ִ��2018.7.1
{
	switch (GetWorkState())	//2018.3.15	ִ�������
	{
		case CHECK_STATE:	//�Լ�ģʽ
		{	//���������ʼ���������Լ�ģʽ //��ʱ����ոտ�������ȴ�һ��ʱ��ȫ���Լ�δ��⵽�쳣��2-3���Լ촥���������ϣ�������Ϊʱ��������Ϊ��ʱ�������㣬���������ʱ����¼
			if(time_1ms_count>10000)	//����LOST״̬�ص�CHECKģʽ����ִ�м����������
			{	//����ִ�е�����˵��LOSTCHECKͨ����������ֵ���
				RC_Calibration();	//self check
				if(1)	//selfcheck��־
				{
					SetWorkState(PREPARE_STATE);	//�˲���ζ�Լ�ͨ����һ��Ӳ��ģ������
				  	//���ݳ�ʼ����
					yunMotorData.pitch_tarP=PITCH_INIT;	//	//��������������̨����
					yunMotorData.yaw_tarP=(ZGyroModuleAngle*10+(YAW_INIT-yunMotorData.yaw_fdbP)*3600/8192);	//�����Ŵ�10������Ŀ��λ����Ϊ�е�
				}
			}
			break;
		}
		case PREPARE_STATE:	//Ԥ��ģʽ
		{	//�ȴ�����״̬�ȶ��������ó�ֵ
			Yun_Task();	//������̨
//			if(abs(Gyro_Data.angvel[0])<20&&abs(Gyro_Data.angvel[2])<20&&abs(yunMotorData.pitch_tarP-(Gyro_Data.angle[0]*8192/360.0f+PITCH_INIT))<50)	//��̨�Ѿ�λ	//λ�û������
		//	if(abs(Gyro_Data.angvel[YAW])<2)	//��̨�Ѿ�λ�����з���
			{
				SetWorkState(CALI_STATE);
			}
			Shoot_Task();	//��ʱ����
			break;
		}
		case CALI_STATE:	//�궨ģʽ
		{
			if(1)	//��Ϊ�ջ��궨	//BulletRotate_OffSetInit()==1		//BulletRotate_Cali()==1&& ��ʱȡ������һϵ�й��ܣ��������·����� 2018.9.9
			{
				SetWorkState(STOP_STATE);
			}
			Yun_Task();	//������̨����
			Shoot_Task();	//��ʱ����
			break;
		}
		case NORMAL_STATE:	//��������ģʽ
		{
			Yun_Task();	//������̨����
			Shoot_Task();
			Friction_Task();
			Remote_Task();
			break;
		}
		case ERROR_STATE:	//����ģʽ
		{
			
			break;
		}
		case LOST_STATE:	//����ģʽ
		{
			break;
		}
		case STOP_STATE:	//ֹͣ״̬
		{
			Remote_Task();
//////			Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
			Yun_Task();	//������̨
		//	Vision_Task(&t_yaw_error,&t_pitch_error);
			break;
		}
		case AUTO_STATE:	//ֹͣ״̬
		{
			Auto_Operation();	//�Զ�����
			
			Yun_Task();	//������̨����
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
		case CHECK_STATE:	//�Լ�ģʽ
		{	//���������ʼ���������Լ�ģʽ //��ʱ����ոտ�������ȴ�һ��ʱ��ȫ���Լ�δ��⵽�쳣��2-3���Լ촥���������ϣ�������Ϊʱ��������Ϊ��ʱ�������㣬���������ʱ����¼
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case PREPARE_STATE:	//Ԥ��ģʽ
		{	//�ȴ�����״̬�ȶ��������ó�ֵ
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case CALI_STATE:	//�궨ģʽ
		{
//			SetFrictionWheelSpeed(FRICTION_INIT);
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case NORMAL_STATE:	//��������ģʽ
		{
			CAN1_Chassis_SendMsg(chassis_Data.lf_wheel_output,chassis_Data.rf_wheel_output,frictionWheel_Data.l_wheel_output,frictionWheel_Data.r_wheel_output);	//Friction_State*8000,Friction_State*-8000
			CAN1_Yun_Shoot_SendMsg(yunMotorData.yaw_output,yunMotorData.pitch_output,shoot_Motor_Data_Down.output,0);
			//CAN1_Yun_Shoot_SendMsg(yunMotorData.yaw_output,0,0,0);	//yaw pitch
			break;
		}
		case ERROR_STATE:	//����ģʽ
		{
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case STOP_STATE:	//ֹͣ״̬
		{
			CAN1_Chassis_SendMsg(0,0,0,0);
			CAN1_Yun_Shoot_SendMsg(0,0,0,0);	//yaw pitch
			break;
		}
		case AUTO_STATE:	//ֹͣ״̬
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
	


void RC_Calibration(void)	//�ϵ���ң��������ֵ����Ĭ�ϲ����Ƚϣ��ж��Ƿ�������������λ
{													//ע���������ң�������ճ�ʼ����
	if(abs(RC_Ctl.rc.ch0+RC_Ctl.rc.ch1+RC_Ctl.rc.ch2+RC_Ctl.rc.ch3-1024*4)>8)
	{
		NVIC_SystemReset();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************��������״̬***********************************************/
RC_Ctl_t RC_DATA_ERROR={0};	//��¼����֡����
void Teleconltroller_Data_protect(void)	//ң���������Ա��� 
{
	u8 protect_state=0xC0;	//��λ��ʾ��ǰң���������Ƿ�����	//���2λΪ����λ����Ϊ1	//364-1024-1684
	protect_state|=(abs(RC_Ctl.rc.ch0-1024)<=662);
	protect_state|=(abs(RC_Ctl.rc.ch1-1024)<=662)<<1;
	protect_state|=(abs(RC_Ctl.rc.ch2-1024)<=662)<<2;
	protect_state|=(abs(RC_Ctl.rc.ch3-1024)<=662)<<3;
	protect_state|=(RC_Ctl.rc.switch_left==1||RC_Ctl.rc.switch_left==2||RC_Ctl.rc.switch_left==3)<<4;
	protect_state|=(RC_Ctl.rc.switch_right==1||RC_Ctl.rc.switch_right==2||RC_Ctl.rc.switch_right==3)<<5;
	
	if(protect_state!=0xFF)	{SetWorkState(LOST_STATE); RC_DATA_ERROR=RC_Ctl;}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////



//�����ֵ����
float MyAbs(float num)
{
	if(num>=0)
		return num;
	else 
		return -num;	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/***********************--����״̬--**********************/
void SetWorkState(WorkState_e state)
{
    workState = state;
}


WorkState_e GetWorkState()
{
	return workState;
}
