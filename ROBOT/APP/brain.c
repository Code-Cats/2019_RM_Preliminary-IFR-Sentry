#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"
#include "friction_wheel.h"
#include "yun.h"

u8 hurt_num=0;
u32 no_hurt_time=0;

s16 bulletSpeedSet=0;
/***************************
哨兵大脑1.0
哨兵模式划分：
1、未发现敌人 缓速全程运行
2、发现步兵，最近点打击
3、发现英雄，至英雄打击死角
***************************/

extern PID_GENERAL          PID_PITCH_POSITION;
extern PID_GENERAL          PID_PITCH_SPEED;
extern PID_GENERAL          PID_YAW_POSITION;
extern PID_GENERAL          PID_YAW_SPEED;

extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;
extern s16 ViceBoard_Position;	//0-2690	//正方向与速度一致
u8 no_enemy_move_state=0;
AUTO_OperationStateTypeDef AutoOperationState=NO_ENEMY;
AUTO_OperationYunStateTypeDef AUTO_OperationYunState=FINDING_ENEMY;


u32 yun_lostenmey_protectcount=0;	//丢失保护
void Auto_Operation(void)	//运行频率1000hz
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
	
	switch(AutoOperationState)	//设置暂停机制  当识别到车时始终有一个count=2000? 目标丢失则count-- 到0则认为目标i丢失 云太太开始中心找  云台地盘用不同标志，将PID切换改到这里执行
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
	switch(AUTO_OperationYunState)	//云台
	{
		case FINDING_ENEMY:
		{
			if(VisionData.armor_sign==1)
			{
				AUTO_OperationYunState=FINDED_ENEMY;
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
			}
			else	//没有目标，云台自动转
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
		case FINDED_ENEMY:	//运行频率1000hz
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

