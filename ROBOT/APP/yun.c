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

void Yun_Task(void)	//��̨�������� 
{
	Record_ImuYawAnglev(imu.angleV.z);
	Record_ImuYawAngle(ZGyroModuleAngle);
	Record_ImuPitchAngle(yunMotorData.pitch_fdbP);
//	if(IMU_Check_Useless_State==0)
	{
		Yun_Control_External_Solution();	//������λ�û�
	}
//	else
//	{
//		Yun_Control_Inscribe_Solution();	//��������λ�÷������ˣ��Ͳ����ٶȻ�����
//	}
}

u8 Yun_Control_RCorPC=RC_CONTROL;
u8 yun_control_pcorrc_last=RC_CONTROL;	//��¼��һ�ο���ģʽ���������л�ʱ��ĳЩ���ݽ��д���	//����ʱ�ܹ����⣬���ļܹ����Բ��øñ���
void Yun_Control_External_Solution(void)	//���÷�������
{
//////	static WorkState_e State_Record=CHECK_STATE;

	if(Yun_Control_RCorPC==PC_CONTROL)
	{	//PC��������
//				PC_Control_Yun(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
	}
	else if(Yun_Control_RCorPC==RC_CONTROL)
	{	//RC��������
		if(RC_Ctl.rc.switch_left==RC_SWITCH_UP||VisionData.vision_control_state==0)
		RC_Control_Yun(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);
	}

	extern float test_pitch;
	test_pitch=0;
	//if(time_1ms_count%100==0)
	//Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);	//���Ƽ�λ�������ڲ�
	
	//ǰ����� ��Ҫ��ʶʱ����
	pitch_Feedforward_to_distinguish();
	
	if(yunMotorData.pitch_tarP-yunMotorData.pitch_fdbP>8192/2)	//�����
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
	
	if(yunMotorData.yaw_tarP-ZGyroModuleAngle*10>1800)	//�����
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
	yunMotorData.yaw_output=PID_General(yunMotorData.yaw_tarV,imu.angleV.z,&PID_YAW_SPEED);	//�������������������
	
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
			yunMotorData.yaw_tarP+=((RC_Ctl.rc.ch2-1024)*45.0/660.0);	//35.0/660.0 ͼ����ʱ���� ��С
			yunMotorData.yaw_tarP=yunMotorData.yaw_tarP>1800?yunMotorData.yaw_tarP-3600:yunMotorData.yaw_tarP;	//�����
			yunMotorData.yaw_tarP=yunMotorData.yaw_tarP<-1800?yunMotorData.yaw_tarP+3600:yunMotorData.yaw_tarP;	//�����
			
			yunMotorData.pitch_tarP+=((RC_Ctl.rc.ch3-1024)*25.0/660.0);	//35.0/660.0 ͼ����ʱ���� ��С
			yunMotorData.pitch_tarP=yunMotorData.pitch_tarP<PITCH_DOWNLIMIT?PITCH_DOWNLIMIT:yunMotorData.pitch_tarP;
			yunMotorData.pitch_tarP=yunMotorData.pitch_tarP>PITCH_UPLIMIT?PITCH_UPLIMIT:yunMotorData.pitch_tarP;
		}
		
		
//		yunMotorData.pitch_tarP=((RC_Ctl.rc.ch3-1024)*1200.0/660.0)+PITCH_INIT;	//-50����Ϊ������ˮƽʱ��̨����	1600-320��-�� 2019.3.13 �е�1500
//		yunMotorData.pitch_tarP=yunMotorData.pitch_tarP<100?100:yunMotorData.pitch_tarP;
//		yunMotorData.pitch_tarP=yunMotorData.pitch_tarP>1650?1650:yunMotorData.pitch_tarP;
}



#define YUN_DOWN_VALUELIMIT 2765	//������λ
#define YUN_UP_VALUELIMIT 3650	//������λ
#define YUN_UP_DISLIMIT 552	//�����Ļ��Χ��UPΪ��
#define YUN_DOWN_DISLIMIT 333	//�����Ļ��Χ��DOWNΪ��

#define YUN_UPMAX_EXTENSION (YUN_UPMAX+200)	//�����Ļ��Χ��UPΪ��
#define YUN_DOWNMAX_EXTENSION (YUN_DOWNMAX+200)	//�����Ļ��Χ��DOWNΪ��


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
�������ƣ�Yaw_Angle_Calculate
�������ܣ�ͨ����ǰ��е�Ƕ�����ֵ��е�ǶȱȽϵõ�ʵ�ʲ��
������������ǰ��е�Ƕȣ�src_angle
          ��ֵ��е�Ƕȣ�Init
��������ֵ��ʵ�ʲ�ǣ�output
��������
****************************************/
float Yaw_Angle_Calculate(int16_t src_angle,int16_t Init)
{
    float output=-(float)(src_angle-Init)/8192*2*PI;	
	  return output;
}
//��¼����ֵ����
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
};	//3.6���

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
};	//��

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
};	//ԭ�棬����
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

s32 Yaw_output_offset(s32 yaw_fbdP)	//�˷���̨yaw������������ǶԳ��ԵĲ��� //��Ȼyaw��̨����������������������Ϊ�����ﲢ��Ϊ��̨�������Ӧ���õ�ǰ����λ��������
{
	s32 offset=0;
	int i=0;
	
	yaw_fbdP=yaw_fbdP>YAW_OFFSET_VALUE[0][0]?YAW_OFFSET_VALUE[0][0]:yaw_fbdP;
	yaw_fbdP=yaw_fbdP<YAW_OFFSET_VALUE[YAW_OFFSET_COUNT-1][0]?YAW_OFFSET_VALUE[YAW_OFFSET_COUNT-1][0]:yaw_fbdP;
	
	for(i=0;i<YAW_OFFSET_COUNT;i++)	//��������Ѱ��λ��
	{
		if(yaw_fbdP>=YAW_OFFSET_VALUE[i][0]) break;
	}
	
	i=i>YAW_OFFSET_COUNT-2?YAW_OFFSET_COUNT-2:i;	//���Ƶ������ڶ���Ԫ�ص�λ�ã�������һ����������Խ��
	
	offset=YAW_OFFSET_VALUE[i][1]+(YAW_OFFSET_VALUE[i+1][1]-YAW_OFFSET_VALUE[i][1])*(YAW_OFFSET_VALUE[i][0]-yaw_fbdP)/(YAW_OFFSET_VALUE[i][0]-YAW_OFFSET_VALUE[i+1][0]);
	return offset;
}

s16 Pitch_output_offset(s32 pitch_tarP)	//�˷���̨pitch������������ǶԳ��ԵĲ���	//��Ϊ��̨pitch���������������������ⲿ���������ֻ����һ����Сֵ�����ʲ���tarP��Ϊ�������տ�����߼�ӷ�Ӧ�ٶ�
{
	s16 offset=0;
//	int i=0;
//	
//	pitch_tarP=pitch_tarP>PITCH_OFFSET_VALUE[0][0]?PITCH_OFFSET_VALUE[0][0]:pitch_tarP;
//	pitch_tarP=pitch_tarP<PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT-1][0]?PITCH_OFFSET_VALUE[PITCH_OFFSET_COUNT-1][0]:pitch_tarP;
//	
//	for(i=0;i<PITCH_OFFSET_COUNT;i++)	//��������Ѱ��λ��
//	{
//		if(pitch_tarP>=PITCH_OFFSET_VALUE[i][0]) break;
//	}
//	
//	i=i>PITCH_OFFSET_COUNT-2?PITCH_OFFSET_COUNT-2:i;	//���Ƶ������ڶ���Ԫ�ص�λ�ã�������һ����������Խ��
//	
//	offset=PITCH_OFFSET_VALUE[i][1]+(PITCH_OFFSET_VALUE[i+1][1]-PITCH_OFFSET_VALUE[i][1])*(PITCH_OFFSET_VALUE[i][0]-pitch_tarP)/(PITCH_OFFSET_VALUE[i][0]-PITCH_OFFSET_VALUE[i+1][0]);
	return offset;
}


////��5810-��7250	//40�ݽ�
//ǰ����ʶ
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

