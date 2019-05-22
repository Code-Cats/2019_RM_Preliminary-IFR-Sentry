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
extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;
extern s16 ViceBoard_Position;	//0-2690	//���������ٶ�һ��
u8 no_enemy_move_state=0;
AUTO_OperationStateTypeDef AutoOperationState=NO_ENEMY;

void Auto_Operation(void)	//����Ƶ��1000hz
{
	static u8 armor_sign_last=0;
	Friction_Speed_Set(21);
	
	if(armor_sign_last==0&&VisionData.armor_sign==1)
	{
		yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
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
	
	
	if(hurt_num!=0)
	{
		AutoOperationState=FIND_INFANTRY;
		no_hurt_time++;
	}
	
	if(no_hurt_time>2000)
	{
		no_hurt_time=0;
		hurt_num=0;
		AutoOperationState=NO_ENEMY;
	}
	
	switch(AutoOperationState)
	{
		case NO_ENEMY:
		{
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(MAX_STROKE-40,360);
				if(ViceBoard_Position>MAX_STROKE-60)
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(40,360);
				if(ViceBoard_Position<60)
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
				Auto_Move_Task(MAX_STROKE-40,460);
				if(ViceBoard_Position>MAX_STROKE-60)
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(40,460);
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
}

