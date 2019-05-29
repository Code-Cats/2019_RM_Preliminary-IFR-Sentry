#include "yun.h"
#include "usart1_remote_analysis.h"
#include "auto_aim.h"

void pitch_Feedforward_to_distinguish(void);
float Pitch_Offset2019(float tarp,float fdbp);
float YawSpeed_Offset2019(float tarv);

s32 YAW_INIT=YAW_INIT_DEFINE;


YUN_MOTOR_DATA 			yunMotorData=YUN_MOTOR_DATA_DEFAULT;
YUN_DATA          	yunData=YUN_DATA_DEFAULT;

PID_GENERAL          PID_PITCH_POSITION=PID_PITCH_POSITION_DEFAULT;
PID_GENERAL          PID_PITCH_SPEED=PID_PITCH_SPEED_DEFAULT;
PID_GENERAL          PID_YAW_POSITION=PID_YAW_POSITION_DEFAULT;
PID_GENERAL          PID_YAW_SPEED=PID_YAW_SPEED_DEFAULT;

extern	RC_Ctl_t RC_Ctl;
extern float ZGyroModuleAngle;
extern IMU_T imu;
extern u32 time_1ms_count;
extern VisionDataTypeDef	VisionData;

void Yun_Task(void)	//云台控制任务 
{
	Record_ImuYawAnglev(imu.angleV.z);
	Record_ImuYawAngle(ZGyroModuleAngle);
	Record_ImuPitchAngle(yunMotorData.pitch_fdbP);
//	if(IMU_Check_Useless_State==0)
	{
		Yun_Control_External_Solution();	//正常的位置环
	}
//	else
//	{
//		Yun_Control_Inscribe_Solution();	//当陀螺仪位置反馈崩了，就采用速度环控制
//	}
}

u8 Yun_Control_RCorPC=RC_CONTROL;
u8 yun_control_pcorrc_last=RC_CONTROL;	//记录上一次控制模式，便于在切换时对某些数据进行处理	//这里时架构问题，更改架构可以不用该变量
void Yun_Control_External_Solution(void)	//外置反馈方案
{
//////	static WorkState_e State_Record=CHECK_STATE;

	if(Yun_Control_RCorPC==PC_CONTROL)
	{	//PC控制数据
//				PC_Control_Yun(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
	}
	else if(Yun_Control_RCorPC==RC_CONTROL)
	{	//RC控制数据
		if(RC_Ctl.rc.switch_left==RC_SWITCH_UP||VisionData.vision_control_state==0)
		RC_Control_Yun(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
	}

	extern float test_pitch;
	test_pitch=0;
	//if(time_1ms_count%100==0)
	//Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);	//控制键位集成再内部
	
	//前馈辨别 需要辨识时再用
	pitch_Feedforward_to_distinguish();
	
	if(yunMotorData.pitch_tarP-yunMotorData.pitch_fdbP>8192/2)	//过零点
	{
		yunMotorData.pitch_tarV=PID_General(yunMotorData.pitch_tarP,yunMotorData .pitch_fdbP+8192,&PID_PITCH_POSITION);
	}
	else if(yunMotorData.pitch_tarP-yunMotorData.pitch_fdbP<-8192/2)
	{
		yunMotorData.pitch_tarV=PID_General(yunMotorData.pitch_tarP,yunMotorData.pitch_fdbP-8192,&PID_PITCH_POSITION);
	}
	else
	{
		yunMotorData.pitch_tarV=PID_General(yunMotorData.pitch_tarP,yunMotorData.pitch_fdbP,&PID_PITCH_POSITION);
	}
	
	if(yunMotorData.yaw_tarP-ZGyroModuleAngle*10>1800)	//过零点
	{
		yunMotorData.yaw_tarV=PID_General(yunMotorData.yaw_tarP,ZGyroModuleAngle*10+3600,&PID_YAW_POSITION);
	}
	else if(yunMotorData.yaw_tarP-ZGyroModuleAngle*10<-1800)
	{
		yunMotorData.yaw_tarV=PID_General(yunMotorData.yaw_tarP,ZGyroModuleAngle*10-3600,&PID_YAW_POSITION);
	}
	else
	{
		yunMotorData.yaw_tarV=PID_General(yunMotorData.yaw_tarP,ZGyroModuleAngle*10,&PID_YAW_POSITION);
	}
	

	
	
	yunMotorData.pitch_output=PID_General(yunMotorData.pitch_tarV,-imu.angleV.y,&PID_PITCH_SPEED);
	yunMotorData.yaw_output=PID_General(yunMotorData.yaw_tarV,imu.angleV.z,&PID_YAW_SPEED);	//采用外界陀螺仪做反馈
	
	yunMotorData.pitch_output+=Pitch_Offset2019(yunMotorData.pitch_tarP,yunMotorData.pitch_tarP);
	yunMotorData.yaw_output+=YawSpeed_Offset2019(yunMotorData.yaw_tarV);
	
	
	yunMotorData.pitch_output=yunMotorData.pitch_output>30000?30000:yunMotorData.pitch_output;
	yunMotorData.pitch_output=yunMotorData.pitch_output<-30000?-30000:yunMotorData.pitch_output;
	
	yunMotorData.yaw_output=yunMotorData.yaw_output>30000?30000:yunMotorData.yaw_output;
	yunMotorData.yaw_output=yunMotorData.yaw_output<-30000?-30000:yunMotorData.yaw_output;
	
//	State_Record=GetWorkState();
}




void RC_Control_Yun(float * yaw_tarp,float * pitch_tarp)	//1000Hz
{
		if(time_1ms_count%15==0)	//66.67hz
		{
			yunMotorData.yaw_tarP+=((RC_Ctl.rc.ch2-1024)*45.0/660.0);	//35.0/660.0 图传延时过大 改小
			yunMotorData.yaw_tarP=yunMotorData.yaw_tarP>1800?yunMotorData.yaw_tarP-3600:yunMotorData.yaw_tarP;	//过零点
			yunMotorData.yaw_tarP=yunMotorData.yaw_tarP<-1800?yunMotorData.yaw_tarP+3600:yunMotorData.yaw_tarP;	//过零点
			
			yunMotorData.pitch_tarP+=((RC_Ctl.rc.ch3-1024)*25.0/660.0);	//35.0/660.0 图传延时过大 改小
			yunMotorData.pitch_tarP=yunMotorData.pitch_tarP<PITCH_DOWNLIMIT?PITCH_DOWNLIMIT:yunMotorData.pitch_tarP;
			yunMotorData.pitch_tarP=yunMotorData.pitch_tarP>PITCH_UPLIMIT?PITCH_UPLIMIT:yunMotorData.pitch_tarP;
		}
		
		
//		yunMotorData.pitch_tarP=((RC_Ctl.rc.ch3-1024)*1200.0/660.0)+PITCH_INIT;	//-50是因为陀螺仪水平时云台上扬	1600-320上-下 2019.3.13 中点1500
//		yunMotorData.pitch_tarP=yunMotorData.pitch_tarP<100?100:yunMotorData.pitch_tarP;
//		yunMotorData.pitch_tarP=yunMotorData.pitch_tarP>1650?1650:yunMotorData.pitch_tarP;
}



#define YUN_DOWN_VALUELIMIT 2765	//向下限位
#define YUN_UP_VALUELIMIT 3650	//向上限位
#define YUN_UP_DISLIMIT 552	//正常的活动范围，UP为正
#define YUN_DOWN_DISLIMIT 333	//正常的活动范围，DOWN为负

#define YUN_UPMAX_EXTENSION (YUN_UPMAX+200)	//补偿的活动范围，UP为负
#define YUN_DOWNMAX_EXTENSION (YUN_DOWNMAX+200)	//补偿的活动范围，DOWN为正


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

s32 yun_pitch_tarV(s32 now_V)
{
	static s32 Last_V=0;
	static s32 now_acc_V=0;
	static float a=0.97f;
	now_acc_V=(s32)(Last_V*(1-a)+now_V*a);
	Last_V=now_acc_V;
	return now_acc_V;
}



/***************************************
函数名称：Yaw_Angle_Calculate
函数功能：通过当前机械角度与中值机械角度比较得到实际差角
函数参数：当前机械角度：src_angle
          中值机械角度：Init
函数返回值：实际差角：output
描述：无
****************************************/
float Yaw_Angle_Calculate(int16_t src_angle,int16_t Init)
{
    float output=-(float)(src_angle-Init)/8192*2*PI;	
	  return output;
}
//记录补偿值曲线
#define YAW_OFFSET_COUNT 11
const s32 YAW_OFFSET_VALUE[YAW_OFFSET_COUNT][2]=\
{\
	{5310,-620},\
	{5200,-530},\
	{5100,-475},\
	{5000,-390},\
	{4900,-110},\
	{4800,70},\
	{4700,280},\
	{4600,400},\
	{4500,470},\
	{4400,530},\
	{4300,573},\
};	//3.6测得

//const s32 YAW_OFFSET_VALUE[YAW_OFFSET_COUNT][2]=\
{\
	{5310,-1000},\
	{5180,-800},\
	{5000,-700},\
	{4910,-600},\
	{4800,-350},\
	{4710,0},\
	{4600,20},\
	{4500,240},\
	{4450,420},\
};	//旧

#define PITCH_OFFSET_COUNT 12
const s32 PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT][2]=\
{\
	{6600,-3500},\
	{6500,-2800},\
	{6400,-2500},\
	{6300,-2300},\
	{6200,-1920},\
	{6100,-1465},\
	{6000,-1300},\
	{5900,-1186},\
	{5800,-1200},\
	{5700,-1000},\
	{5600,-900},\
	{5500,-800},\
};	//原版，过大
//const s32 PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT][2]=\
{\
	{6600,-3000},\
	{6500,-2400},\
	{6400,-2100},\
	{6300,-1900},\
	{6200,-1750},\
	{6100,-1400},\
	{6000,-1200},\
	{5900,-1186},\
	{5800,-1100},\
	{5700,-1000},\
	{5600,-900},\
	{5500,-800},\
};

s32 Yaw_output_offset(s32 yaw_fbdP)	//克服云台yaw轴非线性力及非对称性的补偿 //虽然yaw云台阻尼曲线满足收敛，但因为参照物并非为云台电机，故应采用当前反馈位置做参照
{
	s32 offset=0;
	int i=0;
	
	yaw_fbdP=yaw_fbdP>YAW_OFFSET_VALUE[0][0]?YAW_OFFSET_VALUE[0][0]:yaw_fbdP;
	yaw_fbdP=yaw_fbdP<YAW_OFFSET_VALUE[YAW_OFFSET_COUNT-1][0]?YAW_OFFSET_VALUE[YAW_OFFSET_COUNT-1][0]:yaw_fbdP;
	
	for(i=0;i<YAW_OFFSET_COUNT;i++)	//遍历数组寻找位置
	{
		if(yaw_fbdP>=YAW_OFFSET_VALUE[i][0]) break;
	}
	
	i=i>YAW_OFFSET_COUNT-2?YAW_OFFSET_COUNT-2:i;	//限制到倒数第二个元素的位置，以免下一步运算数组越界
	
	offset=YAW_OFFSET_VALUE[i][1]+(YAW_OFFSET_VALUE[i+1][1]-YAW_OFFSET_VALUE[i][1])*(YAW_OFFSET_VALUE[i][0]-yaw_fbdP)/(YAW_OFFSET_VALUE[i][0]-YAW_OFFSET_VALUE[i+1][0]);
	return offset;
}

s16 Pitch_output_offset(s32 pitch_tarP)	//克服云台pitch轴非线性力及非对称性的补偿	//因为云台pitch阻尼曲线满足收敛（在外部激励情况下只存在一个最小值），故采用tarP作为补偿参照可以提高间接反应速度
{
	s16 offset=0;
//	int i=0;
//	
//	pitch_tarP=pitch_tarP>PITCH_OFFSET_VALUE[0][0]?PITCH_OFFSET_VALUE[0][0]:pitch_tarP;
//	pitch_tarP=pitch_tarP<PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT-1][0]?PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT-1][0]:pitch_tarP;
//	
//	for(i=0;i<PITCH_OFFSET_COUNT;i++)	//遍历数组寻找位置
//	{
//		if(pitch_tarP>=PITCH_OFFSET_VALUE[i][0]) break;
//	}
//	
//	i=i>PITCH_OFFSET_COUNT-2?PITCH_OFFSET_COUNT-2:i;	//限制到倒数第二个元素的位置，以免下一步运算数组越界
//	
//	offset=PITCH_OFFSET_VALUE[i][1]+(PITCH_OFFSET_VALUE[i+1][1]-PITCH_OFFSET_VALUE[i][1])*(PITCH_OFFSET_VALUE[i][0]-pitch_tarP)/(PITCH_OFFSET_VALUE[i][0]-PITCH_OFFSET_VALUE[i+1][0]);
	return offset;
}


////下5810-上7250	//40递进
//前馈辨识
#define AVRNUMS 700
u8 Feedforward_startflag=0;
u8 Feedforward_state=0;
const u16 tapi=6060;
u16 record_i=0;
float fdbpi[40]={0};
float outputi[40]={0};
void pitch_Feedforward_to_distinguish(void)
{
	static u32 timecount=0;
	if(Feedforward_startflag==1)
	{
		
		switch(Feedforward_state)
		{
			case 0:
			{
				yunMotorData.pitch_tarP=tapi+record_i*40;
				if(yunMotorData.pitch_fdbV==0)
				{
					timecount++;
				}
				else
				{
					timecount=0;
				}
				if(timecount>=300)
				{
					timecount=0;
					Feedforward_state=1;
				}
				break;
			}
			case 1:
			{
				fdbpi[record_i]+=yunMotorData.pitch_fdbP;
				outputi[record_i]+=yunMotorData.pitch_output;
				timecount++;
				if(timecount>=AVRNUMS)
				{
					timecount=0;
					Feedforward_state=2;
					fdbpi[record_i]/=AVRNUMS;
					outputi[record_i]/=AVRNUMS;
				}
				break;
			}
			case 2:
			{
				if(record_i<30)
				{
					record_i++;
					Feedforward_state=0;
				}
				else
				{
					Feedforward_startflag=0;
					record_i=0;
					return;
				}
				break;
			}
			
		}
	}
}

float Pitch_Offset2019(float tarp,float fdbp)
{
	float offset=0;
	static u8 laststate=0;
	static float fdbp_last=0;
	
	if(fdbp-fdbp_last>0.000001f)
	{
		laststate=0;
	}
	else if(fdbp-fdbp_last<-0.000001f)
	{
		laststate=1;
	}
	
	if(laststate==0)
	{
		if(tarp<6600)
		{
			offset=-0.03132f*tarp*tarp+413.1f*tarp-1358000;
		}
		else
		{
			offset=0.0125f*tarp*tarp-158.4f*tarp+504300;
		}
	}
	else
	{
		if(tarp<6600)
		{
			offset=-0.03132f*tarp*tarp+413.1f*tarp-1358000;
			offset-=4000;
		}
		else
		{
			offset=0.0125f*tarp*tarp-158.4f*tarp+504300;
			offset-=4000;
		}
	}
	
	offset=offset>13000?13000:offset;
	offset=offset<-13000?-13000:offset;
	
	offset=0;
//	if(offset>-1000&&offset<1000)
//	{
//		offset=0;
//	}
	
	fdbp_last=fdbp;
	
	return offset;
}


float YawSpeed_Offset2019(float tarv)
{
	return 30*tarv;//80
}

