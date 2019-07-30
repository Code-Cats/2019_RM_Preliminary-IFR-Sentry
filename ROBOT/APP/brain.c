#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"
#include "friction_wheel.h"
#include "yun.h"
//#include "usart3_judge_analysis.h"
#include "heat_limit.h"
#include "usart6_viceboard_analysis.h"

#include <stdlib.h>

s16 bulletSpeedSet=0;
/***************************
�ڱ�����1.0
�ڱ�ģʽ���֣�
1��δ���ֵ��� ����ȫ������
2�����ֲ������������
3������Ӣ�ۣ���Ӣ�۴������
***************************/

extern PID_GENERAL          PID_PITCH_POSITION;
extern PID_GENERAL          PID_PITCH_SPEED;
extern PID_GENERAL          PID_YAW_POSITION;
extern PID_GENERAL          PID_YAW_SPEED;

extern ext_game_robot_state_t robot_state_judge;
extern s16 Chassis_Vx;
extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;

#define ORBIT_MAX_POS 2390+40//1740
#define ORBIT_EVADING_DRONES_POSLIMIT 1540//1280 //2000	//������˻���ֹ��
#define ORBIT_ORIGIN_POS 0	//������
#define ORBOT_INFLEXION1_POS 584//500	//��һ���յ�
#define ORBOT_INFLEXION2_POS 1634//1720//1280	//�ڶ����յ�


AUTO_OperationChassisStateTypeDef AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;	//��������
AUTO_OperationYunStateTypeDef AUTO_OperationYunState=YUN_FINDING_ENEMY;	//��������

extern SensorDataTypeDef SensorData;
AutoOperationDataTypeDef AutoOperationData={\
CHASSIS_FINDING_ENEMY_NORMAL,\
YUN_FINDING_ENEMY,\
0,ORBIT_MAX_POS,\
0,\
1,\
160,\
160,\
0,\
};



extern u8 Friction_State;

u8 robot_hurted_state=0;
u32 robot_hurted_num=0;
u16 robot_overpower_count=0;

s8 encoder_caili_flag=0;	//���̱������궨
u32 yun_lostenmey_protectcount=0;	//��ʧ����
void Auto_Operation(void)	//����Ƶ��1000hz
{
	static u8 armor_sign_last=0;
	static u32 robot_hurt_lastnum=0;
	//Friction_Speed_Set(21);
	
	armor_sign_last=VisionData.armor_sign;
	
	//���0.5s
	if(time_1ms_count%500==0)
	{
		static uint16_t last_remain=600;
	
		if(robot_overpower_count>0)
		{
			robot_overpower_count--;
		}
		
		if(robot_state_judge.remain_HP>=400)	//Ѫ������
		{
			if(last_remain-robot_state_judge.remain_HP>=30)	//ȷ����װ���˺�  &&robot_hurt_lastnum!=robot_hurted_num
			{
				robot_hurted_state=1;	//�յ���в��״̬
			}
			else
			{
				robot_hurted_state=0;
			}
		}
		else
		{
			if(last_remain-robot_state_judge.remain_HP>=20)  //&&robot_hurt_lastnum!=robot_hurted_num
			{
				robot_hurted_state=1;
			}
			else
			{
				robot_hurted_state=0;
			}
		}
		
		if(robot_overpower_count>0)
		{
			robot_hurted_state=0;
		}
		
		last_remain=robot_state_judge.remain_HP;
	}
	
	
	/*
		CHASSIS_FINDING_ENEMY_NORMAL,	//����������߱�Ѱ��Ŀ��
	CHASSIS_HIT_ENEMY_STOP,	//���̾�ֹֹͣ����Ŀ��
	CHASSIS_HIT_ENEMY_NEARBY,	//�ͽ������Ŀ��
	CHASSIS_HIT_ENEMY_ELUDE,	//�������ܻ���Ŀ��
	CHASSIS_HIT_ENEMY_WAIST,	//�����Ť����ʽ����(�ܵ����蹥��)
	*/
	static u8 no_enemy_move_state=0;
	switch(AutoOperationState)	//������ͣ����  ��ʶ�𵽳�ʱʼ����һ��count=2000? Ŀ�궪ʧ��count-- ��0����ΪĿ��i��ʧ ��̫̫��ʼ������  ��̨�����ò�ͬ��־����PID�л��ĵ�����ִ��
	{
		case CHASSIS_FINDING_ENEMY_NORMAL://����������߱�Ѱ��Ŀ��
		{
			///*********************************************************************
			AutoOperationData.real_remainbuffer=160;
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(ORBIT_EVADING_DRONES_POSLIMIT,1000);//MAX_STROKE-40
				if(SensorData.encoderPos>+ORBIT_EVADING_DRONES_POSLIMIT-50) //2690-60=2630	MAX_STROKE-60
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,1000);
				if(SensorData.encoderPos<ORBIT_ORIGIN_POS+50)
				{
					no_enemy_move_state=1;
					
				}
			}
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AutoOperationState=CHASSIS_HIT_ENEMY_STOP;
				no_enemy_move_state=!no_enemy_move_state;
			}
//			if(robot_hurted_state==1)	//�ܵ���в ����//////////////////////////////////////////////
//			{
//				//AutoOperationState=CHASSIS_HIT_ENEMY_WAIST;//����һ��ʱ��
//				//AutoOperationState=CHASSIS_HIT_ENEMY_ELUDE;	//���������
//				no_enemy_move_state=!no_enemy_move_state;
//			}
			
			//**************************************************************/
			break;
		}
		case CHASSIS_HIT_ENEMY_STOP:	//���̾�ֹֹͣ����Ŀ��
		{
			Chassis_Vx=0;
			if(yun_lostenmey_protectcount>800)
			{
				AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
			}
			
			if(robot_hurted_state==1)	//�ܵ���ͨ��в
			{
				//AutoOperationState=CHASSIS_HIT_ENEMY_WAIST;//����һ��ʱ��
				///////////////AutoOperationState=CHASSIS_HIT_ENEMY_ELUDE;	//���������
			}//����������յ����ӵ���в
			break;
		}
		case CHASSIS_HIT_ENEMY_NEARBY:	//�ͽ������Ŀ�� ��¼������̨�е� �ж�
		{
			
			break;
		}
//		case CHASSIS_HIT_ENEMY_ELUDE:	//�������ܻ���Ŀ�� ��Զ���ƶ� //��
//		{
//			static u32 timecount=0;
//			static u8 run_pos_state=1;
//			Tar_remain_powerbuffer=90;
//			if(robot_hurt_lastnum!=robot_hurted_num)
//			{
//				u8 nowpos=Recognize_EncoderPosState(ViceBoard_Position);	//����Լ���ǰ�ڹ��λ��
//				if(nowpos==1)	//������
//				{
//					run_pos_state=3;	//����ڽ������յ���� ��ȥԶ���ض�
//				}
//				else if(nowpos==2)
//				{
//					run_pos_state=1;
//				}
//				else
//				{
//					run_pos_state=2;
//				}
//				timecount=0;
//			}
//			
//			Chassis_ORBITState_Run(run_pos_state);
//			
//			timecount++;
//			if(timecount>16000)	//16s ��ʱû�й������˳�
//			{
//				timecount=0;
//				AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
//				Chassis_Vx=0;
//			}
//			
//			break;
//		}
		case CHASSIS_HIT_ENEMY_WAIST:	//�����Ť����ʽ����(�ܵ����蹥��)  ���ܵ�����ֵ�ϴ�ʱ����
		{
			static u32 timecount=0;
			timecount++;
			AutoOperationData.real_remainbuffer=90;
			Auto_Move_Task(ORBIT_ORIGIN_POS,1500);
			
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(ORBIT_MAX_POS,1500);//MAX_STROKE-40
				if(SensorData.encoderPos>ORBIT_EVADING_DRONES_POSLIMIT-40) //2690-60=2630	MAX_STROKE-60
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,1500);
				if(SensorData.encoderPos<ORBIT_ORIGIN_POS+40)
				{
					no_enemy_move_state=1;
					
				}
			}
			
			if(timecount>=2000)
			{
				timecount=0;
				AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
				encoder_caili_flag=1;
			}
				
			
			break;
		}
		default:
			break;
	}
	
/****************************************************8
	if(encoder_caili_flag==1)	//��ԭ��У׼
	{
		Chassis_Vx=-350;
		if(encoder_startsensor==0||encoder_endsensor==0)
		{
			encoder_caili_flag=0;
		}
	}
	if(encoder_caili_flag==-1)
	{
		Chassis_Vx=350;
		if(encoder_startsensor==0||encoder_endsensor==0)
		{
			encoder_caili_flag=0;
		}
	}
	
	if(encoder_startsensor==0&&encoder_endsensor==1)
	{
		Chassis_Vx=350;
	}
	
	if(encoder_endsensor==0&&encoder_startsensor==1)
	{
		Chassis_Vx=-350;
	}
*******************************************************/
	static u8 yun_pitch_findstate=0;
	switch(AUTO_OperationYunState)	//��̨	//������װ���ܵ���������̨ת��״̬-��̨�����ƶ�PID��Ŀ����������ɨ�裬�����ʱ���Ƴ���Ѱ��ģʽ ���sign=1�������ģʽ
	{
		case YUN_FINDING_ENEMY:
		{
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AUTO_OperationYunState=YUN_FINDED_ENEMY;
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
			}
			else	//û��Ŀ�꣬��̨�Զ�ת
			{
				///*********************************************************************************
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10+160;//165
				switch(yun_pitch_findstate)
				{
					case 0:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=2800;
						timecount++;
						if(timecount>250)
						{
							timecount=0;
							yun_pitch_findstate=1;
						}
						
						break;
					}
					case 1:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=2000;
						timecount++;
						if(timecount>250)
						{
							timecount=0;
							yun_pitch_findstate=0;
						}
						break;
					}
				}
				//yunMotorData.pitch_tarP=6700;
				//**********************************************************************************/
			}
			
//			PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//			PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
			PID_PITCH_SPEED.input_max=160;	//18
			PID_PITCH_SPEED.input_min=-160;
			PID_YAW_SPEED.input_max=150;	//15
			PID_YAW_SPEED.input_min=-150;
			break;
		}
		case YUN_FINDED_ENEMY:	//����Ƶ��1000hz
		{
			
			PID_PITCH_SPEED.input_max=80;
			PID_PITCH_SPEED.input_min=-80;
			PID_YAW_SPEED.input_max=100;
			PID_YAW_SPEED.input_min=-100;

			
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				yun_lostenmey_protectcount=0;
			}
			else
			{
				yun_lostenmey_protectcount++;
			}
			
			if(yun_lostenmey_protectcount>800)
			{
				AUTO_OperationYunState=YUN_FINDING_ENEMY;
			}
			
			///**********************************************************************************************************
			if(time_1ms_count%80==0&&yun_lostenmey_protectcount<400&&Shoot_Heat_Limit()==1&&Friction_State==1)
			{
				AddBulletToShootingSystem();
			}
			//***********************************************************************************************************/
			break;
		}
	}
	
	robot_hurt_lastnum=robot_hurted_num;
}

u8 Recognize_EncoderPosState(s16 pos)	//ʶ�𵽵��ǵڼ��׶�
{
	if(pos<=ORBOT_INFLEXION1_POS)
	{
		return 1;
	}
	else if(pos<=ORBOT_INFLEXION2_POS)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

void Chassis_ORBITState_Run(u8 runstate)	//���ݴ�����������ڹ����һ������
{
	switch(runstate)
	{
		case 1:	//�����һ��
		{
			static u8 movestate=0;
			if(movestate==1)
			{
				Auto_Move_Task(ORBOT_INFLEXION1_POS,1000);//MAX_STROKE-40
				if(SensorData.encoderPos>ORBOT_INFLEXION1_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,1000);
				if(SensorData.encoderPos<ORBIT_ORIGIN_POS+70)
				{
					movestate=1;
					
				}
			}
			break;
		}
		case 2:
		{
			static u8 movestate=0;
			if(movestate==1)
			{
				Auto_Move_Task(ORBOT_INFLEXION2_POS,1000);//MAX_STROKE-40
				if(SensorData.encoderPos>ORBOT_INFLEXION2_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBOT_INFLEXION1_POS,1000);
				if(SensorData.encoderPos<ORBOT_INFLEXION1_POS+60)
				{
					movestate=1;
					
				}
			}
			break;
		}
		case 3:
		{
			static u8 movestate=0;
			if(movestate==1)
			{
				Auto_Move_Task(ORBIT_MAX_POS,1000);//MAX_STROKE-40
				if(SensorData.encoderPos>ORBIT_MAX_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBOT_INFLEXION2_POS,1000);
				if(SensorData.encoderPos<ORBOT_INFLEXION2_POS+60)
				{
					movestate=1;
					
				}
			}
			break;
		}
	}
}

extern ext_game_state_t game_state_judge;
extern ext_power_heat_data_t heat_data_judge;

u8 Enemy_drone_state=0;	//0Ϊ������
///@bierf �����Զ��ڱ����� ��������˶� �Զ��ͽ����
///@parm void
///@return void
void Auto_Operation_New(void)	//��Ҫ��������ʣ���ӵ������������Զ��ر�Ħ���֣�����������������ʼ�Զ�����Ħ���֣���ʱ������
{
	static u32 robot_hits_count_last=0;
	
	switch(AutoOperationData.chassis_state)
	{
		case CHASSIS_FINDING_ENEMY_NORMAL:	//Ѱ�ҵ��� 
		{
			Auto_Move_Normal();
			if(VisionData.armor_sign==1)
			{
				AutoOperationData.chassis_state=CHASSIS_HIT_ENEMY_NEARBY;
			}
//			
			if(AutoOperationData.robot_hits_count!=robot_hits_count_last)	//�ܵ����
			{
				AutoOperationData.chassis_state=CHASSIS_HIT_ENEMY_WAIST;
			}
			break;
		}
		case CHASSIS_HIT_ENEMY_STOP:	//��ֹ����
		{
			AutoOperationData.chassis_enable=0;
			if(AutoOperationData.robot_hits_count!=robot_hits_count_last)	//�ܵ����
			{
				AutoOperationData.chassis_state=CHASSIS_HIT_ENEMY_WAIST;
			}
			
			if(AutoOperationData.yun_lost_count>800)
			{
				AutoOperationData.chassis_state=CHASSIS_FINDING_ENEMY_NORMAL;
			}
			break;
		}
		case CHASSIS_HIT_ENEMY_NEARBY:	//�ͽ�����
		{
			AutoOperationData.chassis_state=CHASSIS_HIT_ENEMY_STOP;	//�������л�����
			break;
		}
		case CHASSIS_HIT_ENEMY_WAIST:	//�������
		{
			static u32 timeout=0;
			timeout++;
			Auto_Move_Evade();	//�л�ʱ��Ҫ����λrand state
			
			if(timeout>10000)	//10s
			{
				timeout=0;
				AutoOperationData.chassis_state=CHASSIS_FINDING_ENEMY_NORMAL;	//
			}
			break;
		}
	}
	
	
	static u32 speedout_time=0;
	if(AutoOperationData.robot_hits_count!=robot_hits_count_last)	//�ܵ����
	{
		speedout_time=3000;
		AutoOperationData.tar_remainbuffer=3;
	}
	robot_hits_count_last=AutoOperationData.robot_hits_count;
	if(speedout_time>0)
	{
		speedout_time--;
	}
	if(speedout_time==0)
	{
		AutoOperationData.tar_remainbuffer=160;
	}
	else if(speedout_time==1)
	{
		if(heat_data_judge.chassis_power_buffer>50)
		{
			AutoOperationData.real_remainbuffer=heat_data_judge.chassis_power_buffer-50;		//���ʲ��˷�
		}
	}
	
	if(time_1ms_count%50==0)	//����˲�
	{
		if(AutoOperationData.real_remainbuffer<AutoOperationData.tar_remainbuffer)
		{
			if(time_1ms_count%200==0)
			{
				AutoOperationData.real_remainbuffer++;
			}
		}
		else if(AutoOperationData.real_remainbuffer>AutoOperationData.tar_remainbuffer)
		{
			AutoOperationData.real_remainbuffer=AutoOperationData.tar_remainbuffer;
		}
	}
	
	
	
	static u8 yun_pitch_findstate=0;
	switch(AutoOperationData.yun_state)
	{
		case YUN_FINDING_ENEMY:
		{
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AutoOperationData.yun_state=YUN_FINDED_ENEMY;
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
			}
			else	//û��Ŀ�꣬��̨�Զ�ת
			{
				///*********************************************************************************
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10+160;//165
				switch(yun_pitch_findstate)
				{
					case 0:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=2800;
						timecount++;
						if(timecount>250)
						{
							timecount=0;
							yun_pitch_findstate=1;
						}
						
						break;
					}
					case 1:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=2000;
						timecount++;
						if(timecount>250)
						{
							timecount=0;
							yun_pitch_findstate=0;
						}
						break;
					}
				}
				//yunMotorData.pitch_tarP=6700;
				//**********************************************************************************/
			}
			
//			PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//			PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
			PID_PITCH_SPEED.input_max=170;	//18
			PID_PITCH_SPEED.input_min=-170;
			PID_YAW_SPEED.input_max=150;	//15
			PID_YAW_SPEED.input_min=-150;
			break;
		}
		case YUN_FINDED_ENEMY:	//����Ƶ��1000hz
		{
			
			PID_PITCH_SPEED.input_max=80;
			PID_PITCH_SPEED.input_min=-80;
			PID_YAW_SPEED.input_max=100;
			PID_YAW_SPEED.input_min=-100;

			
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AutoOperationData.yun_lost_count=0;
			}
			else
			{
				AutoOperationData.yun_lost_count++;
			}
			
			if(AutoOperationData.yun_lost_count>800)
			{
				AutoOperationData.yun_state=YUN_FINDING_ENEMY;
			}
			
			///**********************************************************************************************************
			if(time_1ms_count%80==0&&AutoOperationData.yun_lost_count<400&&Shoot_Heat_Limit()==1&&Friction_State==1)
			{
				AddBulletToShootingSystem();
			}
			//***********************************************************************************************************/
			break;
		}
	}
}



//encoder_speed
//extern u8 encoder_startsensor;
//extern u8 encoder_endsensor;
void Auto_Move_Normal(void)	//��ͨģʽȫ�̹����
{
	static u8 normal_move_flag=0;
	switch(normal_move_flag)
	{
		case 0:
		{
			if(SensorData.encoderPos>0)
			{
				Chassis_Vx=-800;
			}
			if(SensorData.switch_start==0||SensorData.encoderPos<100)
			{
				AutoOperationData.chassis_enable=0;
			}
			else
			{
				AutoOperationData.chassis_enable=1;
			} 
			
			if(SensorData.encoderSpeed>-10&&SensorData.encoderPos<200)
			{
				if(Enemy_drone_state==0)
				{
					normal_move_flag=1;
				}
				else
				{
					normal_move_flag=2;
				}
				
			}
			//Auto_Move_Task(ORBIT_EVADING_DRONES_POSLIMIT,1000);//MAX_STROKE-40
			break;
		}
		case 1:
		{
			if(SensorData.encoderPos<2430)
			{
				Chassis_Vx=800;
			}
			if(SensorData.switch_end==0||SensorData.encoderPos>2330)
			{
				AutoOperationData.chassis_enable=0;
			}
			else
			{
				AutoOperationData.chassis_enable=1;
			}
			
			if(SensorData.encoderSpeed<10&&SensorData.encoderPos>2230)
			{
				normal_move_flag=0;
			}
			break;
		}
		case 2:
		{
			if(SensorData.encoderPos<ORBIT_EVADING_DRONES_POSLIMIT-40)
			{
				Chassis_Vx=800;
			}
			
			if(ORBIT_EVADING_DRONES_POSLIMIT-40-SensorData.encoderPos<300)
			{
				AutoOperationData.chassis_enable=0;
				if(SensorData.encoderSpeed<40||ORBIT_EVADING_DRONES_POSLIMIT-40-SensorData.encoderPos<60)//||ABS(ORBIT_EVADING_DRONES_POSLIMIT-SensorData.encoderPos)<50
				{
					normal_move_flag=0;
				}
			}
			else
			{
				AutoOperationData.chassis_enable=1;
			}
				
			break;
		}
	}
}


//encoder_speed
//extern u8 encoder_startsensor;
//extern u8 encoder_endsensor;
#define EVADE_VX 900
void Auto_Move_Evade(void)	//��ģʽ�����
{
	static u8 move_evade_state=4;
	switch(move_evade_state)
	{
		case 0:	//��Ŀ�꿿��
		{
			AutoOperationData.chassis_enable=1;	//ʧ��
			if(AutoOperationData.tar_chassispos>SensorData.encoderPos)
			{
				Chassis_Vx=EVADE_VX;
			}
			else if(AutoOperationData.tar_chassispos<SensorData.encoderPos)
			{
				Chassis_Vx=-EVADE_VX;
			}
			
			if(ABS(AutoOperationData.tar_chassispos-SensorData.encoderPos)<350)
			{
				if(AutoOperationData.tar_chassispos==0)
				{
						//��β����ģʽ
					move_evade_state=1;
				}
				else if(AutoOperationData.tar_chassispos==ORBIT_MAX_POS)
				{
						//��β����ģʽ
					move_evade_state=2;
				}
				else
				{
						//��ͨ����ģʽ
					move_evade_state=3;
				}
			}
			
			break;
		}
		case 1:	//ͷ����
		{
			if(SensorData.encoderPos>0)
			{
				Chassis_Vx=-EVADE_VX;
			}
//			if(SensorData.switch_start==0)	//����
//			{
//				AutoOperationData.chassis_enable=0;
//			}
//			else
//			{
//				AutoOperationData.chassis_enable=1;
//			}
			AutoOperationData.chassis_enable=0;
			
			if(SensorData.encoderSpeed>=-10)//0
			{
				move_evade_state=4;
			}
			break;
		}
		case 2:	//β����  ORBIT_MAX_POS
		{
			
			if(SensorData.encoderPos<ORBIT_MAX_POS)
			{
				Chassis_Vx=EVADE_VX;
			}
//			if(SensorData.switch_end==0)	//����
//			{
//				AutoOperationData.chassis_enable=0;
//			}
//			else
//			{
//				AutoOperationData.chassis_enable=1;
//			}
			AutoOperationData.chassis_enable=0;
			
			if(SensorData.encoderSpeed<=10)//0
			{
				move_evade_state=4;
			}
			break;
		}
		case 3:	//��;����
		{
			AutoOperationData.chassis_enable=0;	//ʧ��
			if(AutoOperationData.last_dir==0)	//Ŀ���Զ����
			{
				if(SensorData.encoderPos >= AutoOperationData.tar_chassispos)
				{
					move_evade_state=4;
				}
			}
			else	//Ŀ��ӽ���Զ
			{
				if(SensorData.encoderPos <= AutoOperationData.tar_chassispos)
				{
					move_evade_state=4;
				}
			}
			
			if(ABS(SensorData.encoderSpeed)<40)
			{
				move_evade_state=4;
			}
			break;
		}
		case 4:	//������·���
		{
			srand(time_1ms_count);	//�����������
			//����500����
			randagain:AutoOperationData.tar_chassispos=rand()%(AutoOperationData.limitX2-AutoOperationData.limitX1)+AutoOperationData.limitX1;	//�õ�X1-X2�����
			if(ABS(AutoOperationData.tar_chassispos-SensorData.encoderPos)<600)
			{
				goto randagain;
			}
			
			if(ABS(ORBIT_MAX_POS-AutoOperationData.tar_chassispos)<250)
			{
				AutoOperationData.tar_chassispos=ORBIT_MAX_POS;
			}
			else if(AutoOperationData.tar_chassispos<250)
			{
				AutoOperationData.tar_chassispos=0;
			}
			
			if(AutoOperationData.tar_chassispos>SensorData.encoderPos)
			{
				AutoOperationData.last_dir=1;
			}
			else
			{
				AutoOperationData.last_dir=0;
			}
			
			move_evade_state=0;
			break;
		}
	}
}


void Avoid_drones_MsgDeal(robot_interactive_data_t* interactive_data_judge)
{
	if(interactive_data_judge->frame_header.data_cmd_id==0x0200)
	{
		if(interactive_data_judge->data[0]==0xfe)
		{
			if(interactive_data_judge->data[1]==0x5a)	//�з����˻����
			{
				Enemy_drone_state=1;
				AutoOperationData.limitX2=ORBIT_EVADING_DRONES_POSLIMIT;
			}
			else if(interactive_data_judge->data[1]==0xa5)	//�з����˻�����
			{
				Enemy_drone_state=0;
				AutoOperationData.limitX2=ORBIT_MAX_POS;
			}
		}
	}
}

/*
uint8_t armor_id;  //��Ѫ���仯����Ϊװ���˺�������װ�� ID��������ֵΪ 0-4 �Ŵ�������˵����װ��Ƭ������Ѫ���仯���ͣ��ñ�����ֵΪ 0
uint8_t hurt_type; //Ѫ���仯����  0x0 װ���˺���Ѫ�� 0x1 ģ����߿�Ѫ�� 0x2 ��ǹ��������Ѫ�� 0x3 �����̹��ʿ�Ѫ
*/
void RobotHurtCallback(uint8_t armorid,uint8_t hurttype)	//����̨ת������
{
	if(hurttype==0)
	{
		AutoOperationData.robot_hits_count++;
	}
	
	//robot_hurted_state=hurttype;
	if(hurttype==0x3)
	{
		robot_overpower_count=4;
	}
}

