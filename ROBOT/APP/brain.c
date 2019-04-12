#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"
#include "friction_wheel.h"
#include "yun.h"


s16 bulletSpeedSet=0;
/***************************
哨兵大脑1.0
哨兵模式划分：
1、未发现敌人 缓速全程运行
2、发现步兵，最近点打击
3、发现英雄，至英雄打击死角
***************************/
extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;
extern s16 ViceBoard_Position;	//0-2690	//正方向与速度一致
u8 no_enemy_move_state=0;
AUTO_OperationStateTypeDef AutoOperationState=NO_ENEMY;

void Auto_Operation(void)
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
		yunMotorData.pitch_tarP=900;
		yunMotorData.yaw_tarP=ZGyroModuleAngle*10+450;
	}
	
	armor_sign_last=VisionData.armor_sign;
	
	switch(AutoOperationState)
	{
		case NO_ENEMY:
		{
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(2690-40,400);
				if(ViceBoard_Position>2690-60)
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(40,400);
				if(ViceBoard_Position<60)
				{
					no_enemy_move_state=1;
					
				}
			}
			break;
		}
		case FIND_INFANTRY:
		{
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

