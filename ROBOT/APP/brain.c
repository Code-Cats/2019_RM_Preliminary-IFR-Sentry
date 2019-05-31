#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"
#include "friction_wheel.h"
#include "yun.h"
#include "usart3_judge_analysis.h"
#include "heat_limit.h"

s16 bulletSpeedSet=0;
/***************************
哨兵大脑1.0
哨兵模式划分：
1、未发现敌人 缓速全程运行
2、发现步兵，最近点打击
3、发现英雄，至英雄打击死角
***************************/

extern u8 encoder_startsensor;
extern u8 encoder_endsensor;

extern PID_GENERAL          PID_PITCH_POSITION;
extern PID_GENERAL          PID_PITCH_SPEED;
extern PID_GENERAL          PID_YAW_POSITION;
extern PID_GENERAL          PID_YAW_SPEED;

extern ext_game_robot_state_t robot_state_judge;
extern s16 Chassis_Vx;
extern u8 Tar_remain_powerbuffer;	//能量缓冲目标值
extern VisionDataTypeDef	VisionData;
 extern float ZGyroModuleAngle;
extern YUN_MOTOR_DATA 			yunMotorData;
extern s16 ViceBoard_Position;	//0-2690	//正方向与速度一致
AUTO_OperationStateTypeDef AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
AUTO_OperationYunStateTypeDef AUTO_OperationYunState=YUN_FINDING_ENEMY;

#define ORBIT_MAX_POS 1740
#define ORBIT_EVADING_DRONES_POSLIMIT 1280 //2000	//躲避无人机中止点
#define ORBIT_ORIGIN_POS 0	//轨道起点
#define ORBOT_INFLEXION1_POS 500	//第一个拐点
#define ORBOT_INFLEXION2_POS 1280	//第二个拐点

extern u8 Friction_State;

u8 robot_hurted_state=0;
u32 robot_hurted_num=0;
u16 robot_overpower_count=0;

s8 encoder_caili_flag=0;	//底盘编码器标定
u32 yun_lostenmey_protectcount=0;	//丢失保护
void Auto_Operation(void)	//运行频率1000hz
{
	static u8 armor_sign_last=0;
	static u32 robot_hurt_lastnum=0;
	//Friction_Speed_Set(21);
	
	armor_sign_last=VisionData.armor_sign;
	
	//检测0.5s
	if(time_1ms_count%500==0)
	{
		static uint16_t last_remain=600;
	
		if(robot_overpower_count>0)
		{
			robot_overpower_count--;
		}
		
		if(robot_state_judge.remain_HP>=400)	//血量充足
		{
			if(last_remain-robot_state_judge.remain_HP>=30)	//确认是装甲伤害  &&robot_hurt_lastnum!=robot_hurted_num
			{
				robot_hurted_state=1;	//收到威胁的状态
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
		CHASSIS_FINDING_ENEMY_NORMAL,	//正常情况边走边寻找目标
	CHASSIS_HIT_ENEMY_STOP,	//底盘静止停止击打目标
	CHASSIS_HIT_ENEMY_NEARBY,	//就近点击打目标
	CHASSIS_HIT_ENEMY_ELUDE,	//优先闪避击打目标
	CHASSIS_HIT_ENEMY_WAIST,	//以随机扭腰方式闪避(受到大弹丸攻击)
	*/
	static u8 no_enemy_move_state=0;
	switch(AutoOperationState)	//设置暂停机制  当识别到车时始终有一个count=2000? 目标丢失则count-- 到0则认为目标i丢失 云太太开始中心找  云台地盘用不同标志，将PID切换改到这里执行
	{
		case CHASSIS_FINDING_ENEMY_NORMAL://正常情况边走边寻找目标
		{
			/*********************************************************************
			Tar_remain_powerbuffer=160;
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(ORBIT_EVADING_DRONES_POSLIMIT,280);//MAX_STROKE-40
				if(ViceBoard_Position>+ORBIT_EVADING_DRONES_POSLIMIT-50) //2690-60=2630	MAX_STROKE-60
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,280);
				if(ViceBoard_Position<ORBIT_ORIGIN_POS+50)
				{
					no_enemy_move_state=1;
					
				}
			}
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AutoOperationState=CHASSIS_HIT_ENEMY_STOP;
				no_enemy_move_state=!no_enemy_move_state;
			}
			if(robot_hurted_state==1)	//受到威胁 进入//////////////////////////////////////////////
			{
				//AutoOperationState=CHASSIS_HIT_ENEMY_WAIST;//猛跑一段时间
				AutoOperationState=CHASSIS_HIT_ENEMY_ELUDE;	//换轨道段跑
				no_enemy_move_state=!no_enemy_move_state;
			}
			
			**************************************************************/
			break;
		}
		case CHASSIS_HIT_ENEMY_STOP:	//底盘静止停止击打目标
		{
			Chassis_Vx=0;
			if(yun_lostenmey_protectcount>800)
			{
				AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
			}
			
			if(robot_hurted_state==1)	//受到普通威胁
			{
				//AutoOperationState=CHASSIS_HIT_ENEMY_WAIST;//猛跑一段时间
				AutoOperationState=CHASSIS_HIT_ENEMY_ELUDE;	//换轨道段跑
			}//下面待补充收到大子弹威胁
			break;
		}
		case CHASSIS_HIT_ENEMY_NEARBY:	//就近点击打目标 记录两个云台中点 判断
		{
			
			break;
		}
		case CHASSIS_HIT_ENEMY_ELUDE:	//优先闪避击打目标 在远点移动 //改
		{
			static u32 timecount=0;
			static u8 run_pos_state=1;
			if(robot_hurt_lastnum!=robot_hurted_num)
			{
				u8 nowpos=Recognize_EncoderPosState(ViceBoard_Position);	//辨别自己当前在轨道位置
				if(nowpos==1)	//近基地
				{
					run_pos_state=3;	//如果在近基地收到打击 则去远基地端
				}
				else if(nowpos==2)
				{
					run_pos_state=1;
				}
				else
				{
					run_pos_state=2;
				}
				timecount=0;
			}
			
			Chassis_ORBITState_Run(run_pos_state);
			
			timecount++;
			if(timecount>16000)	//16s 超时没有攻击则退出
			{
				timecount=0;
				AutoOperationState=CHASSIS_FINDING_ENEMY_NORMAL;
				Chassis_Vx=0;
			}
			
			break;
		}
		case CHASSIS_HIT_ENEMY_WAIST:	//以随机扭腰方式闪避(受到大弹丸攻击)  在受到攻击值较大时启用
		{
			static u32 timecount=0;
			timecount++;
			Tar_remain_powerbuffer=90;
			Auto_Move_Task(ORBIT_ORIGIN_POS,400);
			
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(ORBIT_MAX_POS,400);//MAX_STROKE-40
				if(ViceBoard_Position>ORBIT_EVADING_DRONES_POSLIMIT-40) //2690-60=2630	MAX_STROKE-60
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,350);
				if(ViceBoard_Position<ORBIT_ORIGIN_POS+40)
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
	

	if(encoder_caili_flag==1)	//向原点校准
	{
		Chassis_Vx=-280;
		if(encoder_startsensor==0||encoder_endsensor==0)
		{
			encoder_caili_flag=0;
		}
	}
	if(encoder_caili_flag==-1)
	{
		Chassis_Vx=280;
		if(encoder_startsensor==0||encoder_endsensor==0)
		{
			encoder_caili_flag=0;
		}
	}
	
	if(encoder_startsensor==0&&encoder_endsensor==1)
	{
		Chassis_Vx=320;
	}
	
	if(encoder_endsensor==0&&encoder_startsensor==1)
	{
		Chassis_Vx=-320;
	}
	
	static u8 yun_pitch_findstate=0;
	switch(AUTO_OperationYunState)	//云台	//待加入装甲受到攻击则云台转向状态-云台快速移动PID向目标区域上下扫描，如果超时则推出到寻找模式 如果sign=1则进入打击模式
	{
		case YUN_FINDING_ENEMY:
		{
			if(VisionData.armor_sign==1&&VisionData.armor_dis<=750)
			{
				AUTO_OperationYunState=YUN_FINDED_ENEMY;
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10;
			}
			else	//没有目标，云台自动转
			{
				/*********************************************************************************
				yunMotorData.yaw_tarP=ZGyroModuleAngle*10+165;
				switch(yun_pitch_findstate)
				{
					case 0:
					{
						static u32 timecount=0;
						yunMotorData.pitch_tarP=7100;
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
						yunMotorData.pitch_tarP=6150;
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
				**********************************************************************************/
			}
			
//			PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//			PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//			PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
			PID_PITCH_SPEED.input_max=180;	//18
			PID_PITCH_SPEED.input_min=-180;
			PID_YAW_SPEED.input_max=150;	//15
			PID_YAW_SPEED.input_min=-150;
			break;
		}
		case YUN_FINDED_ENEMY:	//运行频率1000hz
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
			
			/**********************************************************************************************************
			if(time_1ms_count%80==0&&yun_lostenmey_protectcount<400&&Shoot_Heat_Limit()==1&&Friction_State==1)
			{
				AddBulletToShootingSystem();
			}
			***********************************************************************************************************/
			break;
		}
	}
	
	robot_hurt_lastnum=robot_hurted_num;
}

u8 Recognize_EncoderPosState(s16 pos)	//识别到底是第几阶段
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

void Chassis_ORBITState_Run(u8 runstate)	//根据传入参数决定在轨道哪一段运行
{
	switch(runstate)
	{
		case 1:	//轨道第一段
		{
			static u8 movestate=0;
			if(movestate==1)
			{
				Auto_Move_Task(ORBOT_INFLEXION1_POS,320);//MAX_STROKE-40
				if(ViceBoard_Position>ORBOT_INFLEXION1_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBIT_ORIGIN_POS,320);
				if(ViceBoard_Position<ORBIT_ORIGIN_POS+70)
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
				Auto_Move_Task(ORBOT_INFLEXION2_POS,320);//MAX_STROKE-40
				if(ViceBoard_Position>ORBOT_INFLEXION2_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBOT_INFLEXION1_POS,320);
				if(ViceBoard_Position<ORBOT_INFLEXION1_POS+60)
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
				Auto_Move_Task(ORBIT_MAX_POS,320);//MAX_STROKE-40
				if(ViceBoard_Position>ORBIT_MAX_POS-60) //2690-60=2630	MAX_STROKE-60
				{
					movestate=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(ORBOT_INFLEXION2_POS,320);
				if(ViceBoard_Position<ORBOT_INFLEXION2_POS+60)
				{
					movestate=1;
					
				}
			}
			break;
		}
	}
}

/*
uint8_t armor_id;  //当血量变化类型为装甲伤害，代表装甲 ID，其中数值为 0-4 号代表机器人的五个装甲片，其他血量变化类型，该变量数值为 0
uint8_t hurt_type; //血量变化类型  0x0 装甲伤害扣血； 0x1 模块掉线扣血； 0x2 超枪口热量扣血； 0x3 超底盘功率扣血
*/
void RobotHurtCallback(uint8_t armorid,uint8_t hurttype)	//给云台转方向用
{
	if(hurttype==0)
	{
		robot_hurted_num++;
	}
	
	//robot_hurted_state=hurttype;
	if(hurttype==0x3)
	{
		robot_overpower_count=4;
	}
}

