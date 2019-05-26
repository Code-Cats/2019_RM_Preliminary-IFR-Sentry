#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"
#include "friction_wheel.h"
#include "yun.h"

u8 hurt_num=0;
u32 no_hurt_time=0;

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

extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;
extern s16 ViceBoard_Position;	//0-2690	//���������ٶ�һ��
u8 no_enemy_move_state=0;
AUTO_OperationStateTypeDef AutoOperationState=NO_ENEMY;
AUTO_OperationYunStateTypeDef AUTO_OperationYunState=FINDING_ENEMY;


u32 yun_lostenmey_protectcount=0;	//��ʧ����
void Auto_Operation(void)	//����Ƶ��1000hz
{
	static u8 armor_sign_last=0;
	Friction_Speed_Set(21);
	
	if(armor_sign_last==0&&VisionData.armor_sign==1)
	{
		//yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
	}
	
	if(VisionData.armor_sign==1)
	{
		
	}
	else
	{
		//yunMotorData.pitch_tarP=6800;
		//yunMotorData.yaw_tarP=ZGyroModuleAngle*10+450;
	}
	
	armor_sign_last=VisionData.armor_sign;
	
	
//	if(hurt_num!=0)
//	{
//		AutoOperationState=FIND_INFANTRY;
//		no_hurt_time++;
//	}
//	
//	if(no_hurt_time>2000)
//	{
//		no_hurt_time=0;
//		hurt_num=0;
//		AutoOperationState=NO_ENEMY;
//	}
	
	switch(AutoOperationState)	//������ͣ����  ��ʶ�𵽳�ʱʼ����һ��count=2000? Ŀ�궪ʧ��count-- ��0����ΪĿ��i��ʧ ��̫̫��ʼ������  ��̨�����ò�ͬ��־����PID�л��ĵ�����ִ��
	{
		case NO_ENEMY:
		{
			if(no_enemy_move_state==1)
			{
				//Auto_Move_Task(2000,200);//MAX_STROKE-40
				if(ViceBoard_Position>1970) //2690-60=2630	MAX_STROKE-60
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				//Auto_Move_Task(40,200);
				if(ViceBoard_Position<70)
				{
					no_enemy_move_state=1;
					
				}
			}
			break;
		}
		case FIND_INFANTRY:
		{
			if(no_enemy_move_state==1)
			{
				//Auto_Move_Task(MAX_STROKE-40,460);
				if(ViceBoard_Position>MAX_STROKE-60)
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				//Auto_Move_Task(40,460);
				if(ViceBoard_Position<60)
				{
					no_enemy_move_state=1;
					
				}
			}
			break;
		}
		case FINE_HERO:
		{
			break;
		}
		default:
			break;
	}
	
	static u8 yun_pitch_findstate=0;
	switch(AUTO_OperationYunState)	//��̨
	{
		case FINDING_ENEMY:
		{
			if(VisionData.armor_sign==1)
			{
				AUTO_OperationYunState=FINDED_ENEMY;
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
			}
			else	//û��Ŀ�꣬��̨�Զ�ת
			{
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10+180;
				switch(yun_pitch_findstate)
				{
					case 0:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=7160;
						timecount++;
						if(timecount>220)
						{
							timecount=0;
							yun_pitch_findstate=1;
						}
						
						break;
					}
					case 1:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=6300;
						timecount++;
						if(timecount>220)
						{
							timecount=0;
							yun_pitch_findstate=0;
						}
						break;
					}
				}
			}
			
//			PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//			PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
			PID_PITCH_SPEED.input_max=190;
			PID_PITCH_SPEED.input_min=-190;
			PID_YAW_SPEED.input_max=150;
			PID_YAW_SPEED.input_min=-150;
			break;
		}
		case FINDED_ENEMY:	//����Ƶ��1000hz
		{
			
			PID_PITCH_SPEED.input_max=80;
			PID_PITCH_SPEED.input_min=-80;
			PID_YAW_SPEED.input_max=100;
			PID_YAW_SPEED.input_min=-100;

			
			if(VisionData.armor_sign==1)
			{
				yun_lostenmey_protectcount=0;
			}
			else
			{
				yun_lostenmey_protectcount++;
			}
			
			if(yun_lostenmey_protectcount>600)
			{
				AUTO_OperationYunState=FINDING_ENEMY;
			}
			break;
		}
	}
}

