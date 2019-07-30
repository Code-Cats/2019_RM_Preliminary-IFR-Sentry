#include "shoot.h"
#include "math.h"
#include "pid.h"
#include "friction_wheel.h"
#include "heat_limit.h"
#include "control.h"
//#include "vision.h"
#include "usart3_judge_analysis.h"

extern ext_bullet_remaining_t bullet_remaining_judge;
extern ext_game_state_t game_state_judge;

SHOOT_DATA shoot_Data_Down=SHOOT_DATA_INIT;
SHOOT_MOTOR_DATA shoot_Motor_Data_Down ={0};

//SHOOT_DATA shoot_Data_Up=SHOOT_DATA_INIT;
//SHOOT_MOTOR_DATA shoot_Motor_Data_Up ={0};

PID_GENERAL   PID_Shoot_Down_Position=PID_SHOOT_POSITION_DEFAULT;
PID_GENERAL   PID_Shoot_Down_Speed=PID_SHOOT_SPEED_DEFAULT;

//PID_GENERAL   PID_Shoot_Up_Position=PID_SHOOT_POSITION_DEFAULT;
//PID_GENERAL   PID_Shoot_Up_Speed=PID_SHOOT_SPEED_DEFAULT;

extern RC_Ctl_t RC_Ctl;

extern u32 time_1ms_count;

extern KeyBoardTypeDef KeyBoardData[KEY_NUMS];

//extern RobotHeatDataSimuTypeDef RobotHeatDataSimu42;
//extern tGameRobotState         testGameRobotState;      //比赛机器人状态
extern u8 Robot_Level;

u8 Friction_State=0;	//初始化不开启
//const u16 FRICTION_INIT=800;
u16 FRICTION_SHOOT=1300;//1640白天;//1470;//1540;	//发弹的PWM	在检录处测的射速13米每秒
u16 Friction_Send=FRICTION_INIT;\

void Shoot_Task(void)	//定时频率：1ms
{
	if(GetWorkState()==AUTO_STATE)
	{
		LASER_SWITCH=0;
		if(bullet_remaining_judge.bullet_remaining_num==0||game_state_judge.game_progress==5)	//无发弹量或者比赛结束关闭摩擦轮  //bullet_remaining_judge.bullet_remaining_num==0||
		{
			frictionWheel_Data.l_wheel_tarV=0;
			Friction_State=0;
		}
	}
	else
	{
		LASER_SWITCH=1; 
		if(Friction_State==1)
		{
			frictionWheel_Data.l_wheel_tarV=18;
		}
		else
		{
			frictionWheel_Data.l_wheel_tarV=0;
		}
	}
	//LASER_SWITCH=Friction_State; 
	
	
	Friction_Speed_Set();
	
	Shoot_Instruction();
	shoot_Motor_Data_Down.tarP=(s32)shoot_Data_Down.motor_tarP;
//	shoot_Motor_Data_Up.tarP=(s32)shoot_Data_Up.motor_tarP;
	
	shoot_Motor_Data_Down.tarV=PID_General(shoot_Motor_Data_Down.tarP,shoot_Motor_Data_Down.fdbP,&PID_Shoot_Down_Position);
//	shoot_Motor_Data_Up.tarV=PID_General(shoot_Motor_Data_Up.tarP,shoot_Motor_Data_Up.fdbP,&PID_Shoot_Up_Position);

	Friction_Send=FRICTION_INIT-(FRICTION_INIT-FRICTION_SHOOT)*Friction_State;	//1888对应射速20,1800-14	1830-14.7	1840-15.1（5.14）	1850最高16，最低15		//经过观察，可能和电压有关系，满电时1860为17.7，空电为15.7

	shoot_Motor_Data_Down.output=PID_General(shoot_Motor_Data_Down.tarV,shoot_Motor_Data_Down.fdbV,&PID_Shoot_Down_Speed);//down

}


u16 shoot_time_record=0;
u16 shoot_time_measure(const s16 tarP,const s16 fbdP,const u8 last_mouse_press_l)
{
	static u8 once_statu=0;
	static u16 time_count_start=0;
	static u16 time_count_end=0;
	static u16 time_count_tem=0;
	if(RC_Ctl.mouse.press_l==1&&last_mouse_press_l==0)
	{
		time_count_tem=time_1ms_count;
		once_statu=0;
	}
	if(ABS(tarP-fbdP)<2&&once_statu!=1)
	{
		once_statu=1;
		time_count_end=time_1ms_count;
		time_count_start=time_count_tem;
	}
	
	return  time_count_end-time_count_start;
}




#define SINGLE_INCREMENT_OLD_2006 196.608f	//8192*96/4/1000	一圈的累加值8192*96除上一圈7个子弹除以编码器转换倍数=发射一颗子弹的位置增量
#define SINGLE_INCREMENT_NEW_2006 18.432f//65.536f		//8192*32/4/1000  8192*36/16/1000=18.432f
#define SINGLE_INCREMENT SINGLE_INCREMENT_NEW_2006	//5.11少
//输出为发弹量，单位颗
//注：应当在本函数或者另一指令解析函数中设置逻辑：切换状态就重置发弹指令（以免突发情况使程序具有滞后性）
//或者将发弹逻辑改为基于增量式方法的频率控制
void Shoot_Instruction(void)	//发弹指令模块
{
	static u8 auto_takebullet_statu_last=0;
//	static WorkState_e State_Record=CHECK_STATE;

	RC_Control_Shoot(&Friction_State);
	
//	if((auto_takebullet_statu_last==0&&auto_takebullet_statu==1)||KeyBoardData[KEY_R].value==1)	//开摩擦轮
//	{
//		Friction_State=1;	//自动开摩擦轮
//	}
//	shoot_time_record=shoot_time_measure(shoot_Data_Down.count,shoot_Data_Down.count_fdb,last_mouse_press_l);////////////////////////////////
////////////	if(time_1ms_count%100==0)
////////////	shoot_Data_Down.count++;
	
	
	shoot_Data_Down.motor_tarP=((float)shoot_Data_Down.count*SINGLE_INCREMENT);	//新2006
//	shoot_Data_Up.motor_tarP=((float)shoot_Data_Up.count*SINGLE_INCREMENT);	//新2006
	

	////////////////////////////Prevent_Jam_Down(&shoot_Data_Down,&shoot_Motor_Data_Down);
//	Prevent_Jam_Up(&shoot_Data_Up,&shoot_Motor_Data_Up);
	
//	State_Record=GetWorkState();
}


u8 HeatLimitState=0;
u8 Shoot_RC_Control_State=1;	//当进行按键操作后，进行RC屏蔽
void RC_Control_Shoot(u8* fri_state)
{
	static u8 swicth_Last_state=0;	//右拨杆
	if(Shoot_RC_Control_State==1)
	{
		HeatLimitState=Shoot_Heat_Limit();
		if(HeatLimitState==1&&frictionWheel_Data.l_wheel_tarV!=0)	//热量限制	Shoot_Heat_Limit()==1&&
		{
			if(RC_Ctl.rc.switch_left!=RC_SWITCH_MIDDLE&&swicth_Last_state==RC_SWITCH_MIDDLE&&RC_Ctl.rc.switch_right==RC_SWITCH_DOWN)
			{
				AddBulletToShootingSystem();
				//shoot_Data_Down.count+=1;
				//shoot_Data_Down.count_float=shoot_Data_Down.count;
				//shoot_Data_Down.last_time=time_1ms_count;
			}
			
			if(time_1ms_count%80==0&&RC_Ctl.rc.switch_left!=RC_SWITCH_MIDDLE&&RC_Ctl.rc.switch_right==RC_SWITCH_DOWN)
			{
//////				if(ABS(RC_Ctl.rc.ch1-1024)>100)	//测试最高射频
//////				{
//////					shoot_Data_Down.count_float+=1;//2;
//////				}
				//shoot_Data_Down.count_float+=(ABS(RC_Ctl.rc.ch1-1024))/400.0f;//2;
				//shoot_Data_Down.count=(s16)shoot_Data_Down.count_float;
				
				if(ABS(RC_Ctl.rc.ch1-1024)>100)	//测试最高射频
				{
					AddBulletToShootingSystem();
				}
				
				//shoot_Data_Down.last_time=time_1ms_count;
			}

		}
		
		if(RC_Ctl.rc.switch_left!=RC_SWITCH_MIDDLE&&swicth_Last_state==RC_SWITCH_MIDDLE&&RC_Ctl.rc.switch_right==RC_SWITCH_UP&&GetWorkState()!=AUTO_STATE)
		{
			*fri_state=!*fri_state;
		}
	}
	swicth_Last_state=RC_Ctl.rc.switch_right;
}

extern s16 Auto_Shoot_Interval_Time;

void AddBulletToShootingSystem(void)
{
	if(shoot_Data_Down.count-shoot_Data_Down.count_fdb<2)
	{
		shoot_Data_Down.count+=1;
		shoot_Data_Down.count_float=shoot_Data_Down.count;
		shoot_Data_Down.last_time=time_1ms_count;
	}
}

#define G 9.80151f
/**********************************
Visual_Pretreatment
deal the Visual data
output:disitance: dm
			 priority:0-10
**********************************/
void Visual_Pretreatment()
{
	shoot_Data_Down.Visual.distance=20;
	shoot_Data_Down.Visual.priority=10;
}

/*********************************
Shoot_Rate_Set
caculate the rate data based on the visual.distance data
用于自动射击
*********************************/
void Shoot_Rate_Set()
{
	
}

/*********************************
Shoot_Frequency_Set
*********************************/
void Shoot_Frequency_Set()
{
	
}


s32 jam_DownfdbP_record;	//这里必须是s32不然在开始时卡单会死循环
#define JAM_FALLBACK 50//34	//100	//往回走的距离
//对tarP的操作
void Prevent_Jam_Down(SHOOT_DATA * shoot_data,SHOOT_MOTOR_DATA * shoot_motor_Data)	//防卡弹程序	//同时包含防鸡蛋的功能	//放在tarP计算出之后
{
	static s32 deviation=0;	//偏差
	static u8 jam_deal_state=0;
//	static u16 ferquency_last=0;
	
	deviation=shoot_motor_Data->tarP-shoot_motor_Data->fdbP;
	
//	if(shoot_data->frequency!=ferquency_last)
//	{
//		shoot_data->Jam.count=0;	//重置count
//	}
//	ferquency_last=shoot_data->frequency;	//迭代
	
	
	if(ABS(deviation)>6&&ABS(shoot_motor_Data->fdbV)<10)	//期望速度不为0时位置未发生变化	//bug:频率刷新时需要刷新count	//手动射击将频率检测删除
	{
		shoot_data->Jam.count++;
	}
	else
	{
		shoot_data->Jam.count=0;
	}
	
//	if(shoot_data->cycle!=0)
//	{
		if(shoot_data->Jam.count>100&&shoot_data->Jam.sign==0)	//超出非正常时间	//且仅执行一次
		{
			 shoot_data->Jam.sign=1;	//标记卡弹
			 jam_deal_state=1;	//标记卡弹处理进程状态
		}
//	}
	
	if(shoot_data->Jam.sign==1)	//处理卡弹模块
	{
		switch (jam_deal_state)
		{
				case 1:
				{
					jam_DownfdbP_record=shoot_motor_Data->fdbP-JAM_FALLBACK;	//可能会在开始时候卡弹有危险？
					shoot_data->motor_tarP=jam_DownfdbP_record;
					jam_deal_state=2;
					break;
				}
				case 2:
				{
					shoot_data->motor_tarP=jam_DownfdbP_record;
					if(ABS(shoot_motor_Data->fdbP-jam_DownfdbP_record)<40)	//认为已经执行了动作	//50
					{
						jam_deal_state=3;
					}
					break;
				}
				case 3:
				{
					shoot_data->Jam.sign=0;	//Reset
					jam_deal_state=0;	//
					shoot_data->count=shoot_data->count_fdb;	//重置子弹数据，防止鸡蛋	//？是否需要+-1？
					shoot_data->count_float=shoot_data->count;
					shoot_data->Jam.count=0;	//重置卡弹检测数据，防止误检测
					break;
				}
		}
	}
	
}   





/*****************************************
函数名称：Shoot_Feedback_Deal
函数功能：拨弹电机反馈数据解析+处理

*****************************************/
void Shoot_Feedback_Deal(SHOOT_DATA *shoot_data,SHOOT_MOTOR_DATA *shoot_motor_data,CanRxMsg *msg)
{
	shoot_motor_data->fdbP_raw=(msg->Data[0]<<8)|msg->Data[1];//接收到的真实数据值  处理频率1KHz
	shoot_motor_data->fdbV=(msg->Data[2]<<8)|msg->Data[3];
	
	shoot_motor_data->fdbP_diff=shoot_motor_data->fdbP_raw_last-shoot_motor_data->fdbP_raw;
	if(shoot_motor_data->fdbP_diff>4096)	//按照6倍采样来计算，机械角度共8192个挡位，则过界表现差值为6826
	{																			//注：此函数未对第一次运行时的可能的圈数直接为1的偏差做处理（处理方法在初始化中标定初始角度值）
		shoot_motor_data->fdbP_raw_sum+=8192;
	}
	else if(shoot_motor_data->fdbP_diff<-4096)
	{
		shoot_motor_data->fdbP_raw_sum-=8192;
	}
	
	shoot_motor_data->fdbP=(s32)((shoot_motor_data->fdbP_raw_sum+shoot_motor_data->fdbP_raw)/1000.0f);	//因为2006减速比过大 不便精确
	
	shoot_motor_data->fdbP_raw_last=shoot_motor_data->fdbP_raw;	//数据迭代
	
	shoot_data->count_fdb=(u16)(shoot_motor_data->fdbP/SINGLE_INCREMENT);	///////防止卡弹，刷新反馈的
}




//一圈7个
//极限15小

