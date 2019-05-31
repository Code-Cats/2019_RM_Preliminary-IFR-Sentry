#include "auto_aim.h"
#include "math.h"
#include "yun.h"
#include "protect.h"

//#include "arm_math.h"

//PITCH_GYRO_INIT 2720

extern PID_GENERAL          PID_PITCH_POSITION;
extern PID_GENERAL          PID_PITCH_SPEED;
extern PID_GENERAL          PID_YAW_POSITION;
extern PID_GENERAL          PID_YAW_SPEED;

extern YUN_MOTOR_DATA 			yunMotorData;
extern RC_Ctl_t RC_Ctl;
extern VisionDataTypeDef	VisionData;
//extern GYRO_DATA Gyro_Data;	//�ں���
extern float ZGyroModuleAngle;	
extern IMU_T imu;	////�ں���

float t_yaw_error=0;	//��ʱ����

float t_gravity_ballistic_set_angel=0;	//���������Ƕ�
s16 t_gravity_ballistic_set_angel_10=0;
	
	
void aim_Pos_Recoed()
{
	
}	

	
float Pixel_to_angle(s16 pix_error)	//���������ת���ɽǶ�
{
	float angel_error=0;
angel_error=atan(pix_error/1460.0f)*57.3f;	//arm_atan_f32ΪDSP	//1855��2��ȥ���  �Ӿ��궨����1750
	t_yaw_error=angel_error;
	
	angel_error=angel_error>12?12:angel_error;
	angel_error=angel_error<-12?-12:angel_error;
	//30.79
	//angel_error=pix_error/30.79f;
	
	return angel_error;
}

#define CAMERA_D	1460	//�����Ƭ���й�Ƭ��Ч������
float Pixel_V_to_angle_V(s16 pix_v,s16 pix_error)	//����ԭʼ�����ݽ��м�����Լ��ٵ�Ƭ����������ľ��ȶ�ʧ��������ӣ�
{
	
	int camera_d_2=CAMERA_D*CAMERA_D;	//����ƽ��
	int r_2=camera_d_2+pix_error*pix_error;	//��Ч�뾶ƽ��
	float cos_angel_2=(float)camera_d_2/(float)r_2;
	float angel_v=0;
	angel_v=pix_v*cos_angel_2/(float)CAMERA_D;
	angel_v=angel_v*57.5f;//*2*PI;	//���л�ԭ����
	
	//angel_v=0.036081f*pix_v+0.1f;//+0.3026f;	//MATLAB���
	
	//float angel_v=0;
	return angel_v;
}


//#define PITCH_INIT         7197	//2019.5.22
#define YUN_DOWN_VALUELIMIT 6500	//������λ
#define YUN_UP_VALUELIMIT 7350	//������λ
#define YUN_UP_DISLIMIT 100	//�����Ļ��Χ��UPΪ��
#define YUN_DOWN_DISLIMIT 1500	//�����Ļ��Χ��DOWNΪ��


float Shoot_V=18.0f;//15.5f	//14M/s
float Shoot_V_2= 324.0f;//(SHOOT_V*SHOOT_V)

s32 imu_matchz_10=0;
float pix_anglev=0;
s32 pix_anglev_10=0;
s32 anglev_mix_10;
s32 anglev_mix_10_filter;

s32 imu_angelv_z_10=0;
float yaw_residual_error=0;	//���ƶ���ʱ��̨���澲��
/////////////////
////////////////
u8 sign_count=0;	//����֡�ſ�ʼ��̬ʶ��
#define VISION_TARX 990//1035//710//640+105//1053//1035��������װƫ��1020//580	//����ԭ��	640
#define VISION_TARY	580//570//580//590//625//570//512+60//360//510//490//480//490//500//520//540//560//360//410//440	//����ԭ��	480	//��5����Ŀ�꣺���ϲ���518-360�����ص�	//��Ϊ�������㶨��̬���ʲ���
void Vision_Task(float* yaw_tarP,float* pitch_tarP)	//����Ŀ��Ƕ�
{
	Shoot_V_2=Shoot_V*Shoot_V;
	imu_angelv_z_10=(s32)(imu.angleV.z*10);
	if(Error_Check.statu[LOST_VISION]==1){	VisionData.armor_type=0;VisionData.armor_sign=0;	}//���޷���=����Task�����ж��������У�������yun.c���Խ���Ƶ�ʱ�������
	//t_yaw_angel_v=Pixel_V_to_angle_V(VisionData.pix_x_v,(s16)(VisionData.error_x-VISION_TARX));
//	t_target_v=t_yaw_angel_v+Gyro_Data
	if(RC_Ctl.rc.switch_left==RC_SWITCH_DOWN&&VisionData.armor_sign==1&&VisionData.armor_dis<=750)	//VisionData.armor_sign!=0
	{
		VisionData.vision_control_state=1;	//���տ���λ
//		PID_PITCH_SPEED.input_max=80;
//		PID_PITCH_SPEED.input_min=-80;
//		PID_YAW_SPEED.input_max=100;
//		PID_YAW_SPEED.input_min=-100;
	}
	else
	{
		VisionData.vision_control_state=0;	//���տ���λ
//		PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//		PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//		PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//		PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
	}
	
	if(VisionData.armor_sign==1)	//�����������ǰ��֡�ٶ���0
	{
		if(sign_count<20)
		sign_count++;
	}
	else
	{
		sign_count=0;
	}
	

	VisionData.imu_vz_match= GetRecordYawAnglev(22);
	imu_matchz_10=(s32)(VisionData.imu_vz_match*10);
	pix_anglev=Pixel_V_to_angle_V(VisionData.pix_x_v,(s16)(VisionData.tar_x-VISION_TARX));
	
	if(sign_count>3)	//�ȶ���
	{
		Tar_Relative_V_Mix(VisionData.imu_vz_match,pix_anglev);	//�ٶ��ں�
	}
	else
	{
		Tar_Relative_V_Mix(0,0);	//�ٶ��ں�
	}
	
	if(sign_count<10)
	{
		VisionData.angle_x_v_filter*=sign_count/10.0f;
	}
	
	if(VisionData.vision_control_state==1)
	{
//		t_yaw_error=Pixel_to_angle((s16)(VisionData.error_x-VISION_TARX))*10;
//		t_pitch_error=Pixel_to_angle((s16)(VisionData.error_y-VISION_TARY))*8192/360;
		
//		t_yaw_error=(float)Gyro_Data.angle[2]*10-Pixel_to_angle((s16)(VisionData.error_x-VISION_TARX))*10;
//		t_pitch_error=(float)yunMotorData.pitch_fdbP+Pixel_to_angle((s16)(VisionData.error_y-VISION_TARY))*8192/360;
		float offset_x_angle=atan(9.0f/VisionData.armor_dis)*573;
		float zgyro_match_10=GetRecordYawAngle(10)*10;
		float pitch_fdb_match=GetRecordPitchAngle(10);
		*yaw_tarP=(float)ZGyroModuleAngle*10+Pixel_to_angle((s16)(VisionData.tar_x-VISION_TARX))*10-offset_x_angle;//(float)ZGyroModuleAngle*10
		*pitch_tarP=yunMotorData.pitch_fdbP-Pixel_to_angle((s16)(VisionData.tar_y-VISION_TARY))*8192/360;
		t_gravity_ballistic_set_angel=Gravity_Ballistic_Set(pitch_tarP,(float)(VisionData.armor_dis_filter/100.0f));	//��������
		
		//
		
		
			if(VisionData.armor_dis<600)	//ֻԤ��6m����
			{
				//Tar_Move_Set(yaw_tarP,(float)(VisionData.armor_dis/100.0f),VisionData.angle_x_v_filter);	//Ԥ�� ������
			}

		
		//t_gravity_ballistic_set_angel_10=(s16)(t_gravity_ballistic_set_angel*10);
		
		*pitch_tarP=*pitch_tarP>(PITCH_INIT+YUN_UP_DISLIMIT)?(PITCH_INIT+YUN_UP_DISLIMIT):*pitch_tarP;	//�����г�
		*pitch_tarP=*pitch_tarP<(PITCH_INIT-YUN_DOWN_DISLIMIT)?(PITCH_INIT-YUN_DOWN_DISLIMIT):*pitch_tarP;	//�����г�
		
		yaw_residual_error=*yaw_tarP-ZGyroModuleAngle*10;	//�в��¼
			
			//if(yaw_residual_error)	//�Զ�����
	}
	
}

#define G	9.8f	//�������ٶ�
float Gravity_Ballistic_Set(float* pitch_tarP,float dis_m)	//������������ϵ�У�����Ϊ��
{
	if(dis_m>8)	dis_m=8;
	
	//dis_m-=0.15f;	//��ʱ�ӵģ���Ϊ������װ���·�
//	static float tar_angle_rad_fliter=0;
	float tar_angle_rad=(PITCH_GYRO_INIT-*pitch_tarP)*0.000767f;	//�����Ƽ򻯼���2pi/8192//////////////////////////////////////////
//	tar_angle_rad_fliter=0.9f*tar_angle_rad_fliter+0.1f*tar_angle_rad;
	float sin_tar_angle=sin(tar_angle_rad);
	float gravity_ballistic_angle_rad=0;	//������ ������
	float gravity_ballistic_angle=0;	//������ �Ƕ���
	gravity_ballistic_angle_rad=0.5f*(-asin((G*dis_m*(1-sin_tar_angle*sin_tar_angle)-sin_tar_angle*Shoot_V_2)/Shoot_V_2)+tar_angle_rad);
	gravity_ballistic_angle=gravity_ballistic_angle_rad*57.3f;

	*pitch_tarP=PITCH_GYRO_INIT-gravity_ballistic_angle*8192.0f/360;//////////////////////////////

	return gravity_ballistic_angle;
}


#define PITCHANGLE_REDNUMS 60
float PitchAngleLast[PITCHANGLE_REDNUMS];
u16 PitchAngleLastcount=0;
void Record_ImuPitchAngle(float angle_z)	//��¼pitchλ�����ݵ���Ƶ��1ms POS
{
	PitchAngleLast[PitchAngleLastcount]=angle_z;
	PitchAngleLastcount++;
	if(PitchAngleLastcount>=PITCHANGLE_REDNUMS)
	{
		PitchAngleLastcount=0;
	}
}

float GetRecordPitchAngle(u16 lastcount)	//��ȡ��ȥ��pitchĿ��ֵ POS
{
	lastcount=lastcount>(PITCHANGLE_REDNUMS-1)?(PITCHANGLE_REDNUMS-1):lastcount;
	s16 lastindex=(s16)PitchAngleLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+PITCHANGLE_REDNUMS):lastindex;
	return PitchAngleLast[lastindex];
}


#define YAWANGLE_REDNUMS 60
float YawAngleLast[YAWANGLE_REDNUMS];
u16 YawAngleLastcount=0;
void Record_ImuYawAngle(float angle_z)	//��¼yawλ�����ݵ���Ƶ��1ms POS
{
	YawAngleLast[YawAngleLastcount]=angle_z;
	YawAngleLastcount++;
	if(YawAngleLastcount>=YAWANGLE_REDNUMS)
	{
		YawAngleLastcount=0;
	}
}

float GetRecordYawAngle(u16 lastcount)	//��ȡ��ȥ��yawĿ��ֵ POS
{
	lastcount=lastcount>(YAWANGLE_REDNUMS-1)?(YAWANGLE_REDNUMS-1):lastcount;
	s16 lastindex=(s16)YawAngleLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+YAWANGLE_REDNUMS):lastindex;
	return YawAngleLast[lastindex];
}


#define YAWANGLEV_REDNUMS 50
float YawAnglevLast[YAWANGLEV_REDNUMS];
u16 YawAnglevLastcount=0;
void Record_ImuYawAnglev(float anglev_z)	//��¼���ݵ���Ƶ��1ms
{
	YawAnglevLast[YawAnglevLastcount]=anglev_z;
	YawAnglevLastcount++;
	if(YawAnglevLastcount>=YAWANGLEV_REDNUMS)
	{
		YawAnglevLastcount=0;
	}
}

float GetRecordYawAnglev(u16 lastcount)
{
	lastcount=lastcount>(YAWANGLEV_REDNUMS-1)?(YAWANGLEV_REDNUMS-1):lastcount;
	s16 lastindex=(s16)YawAnglevLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+YAWANGLEV_REDNUMS):lastindex;
	return YawAnglevLast[lastindex];
}


s16 pix_x_v_filter_10=0;
s16 tar_v_ronghe=0;
s16 tar_v_ronghe_filter=0;
float pix_x_v_filter=0;
#define FILTER_FACTOR	3	//5���˲�
void Tar_Relative_V_Mix(float yaw_angvel,s16 pix_x_anglev)
{
	
	if(yaw_angvel>45)	yaw_angvel=45;	//�޷�
	if(yaw_angvel<-45)	yaw_angvel=-45;
	if(pix_x_anglev>45)	pix_x_anglev=45;
	if(pix_x_anglev<-45)	pix_x_anglev=-45;
	
	
				static s16 pix_x_v_last[FILTER_FACTOR-1]={0};	//4����ֵ�˲�
				static u8 last_count=0;
				s16 pix_x_v_now_sort[FILTER_FACTOR];
				
				
				for(u8 i=0;i<FILTER_FACTOR-1;i++)	//copy last data
				{
					  pix_x_v_now_sort[i]=pix_x_v_last[i];
				}
				
				pix_x_v_now_sort[FILTER_FACTOR-1]=pix_x_anglev;	//copy current data
				
				for(u8 i=0;i<FILTER_FACTOR-1;i++)	//ð������	ֻ��ѭ��FILTER_FACTOR-1  FILTER_FACTOR-2  FILTER_FACTOR-3 FILTER_FACTOR-4 ��
				{
					for(u8 j=0;j<FILTER_FACTOR-1-i;j++)
					{
						if(pix_x_v_now_sort[j]<pix_x_v_now_sort[j+1])
						{
							float tem_small=pix_x_v_now_sort[j];
							pix_x_v_now_sort[j]=pix_x_v_now_sort[j+1];
							pix_x_v_now_sort[j+1]=tem_small;
						}
					}
				}	//�������
				
				if(FILTER_FACTOR%2==1)	//��λ��
				{
					pix_x_v_filter=pix_x_v_now_sort[(FILTER_FACTOR-1)/2];
				}
				else
				{
					pix_x_v_filter=(pix_x_v_now_sort[FILTER_FACTOR/2]+pix_x_v_now_sort[FILTER_FACTOR/2-1])/2;
				}//�������

	
	
	static float f_pix_x_v_filter=0;
	
	f_pix_x_v_filter=f_pix_x_v_filter*0.5f+pix_x_v_filter*0.5f;	//6-4
	
	pix_x_v_filter_10=(s16)(f_pix_x_v_filter*10);
	
	


		pix_anglev_10=(s32)(f_pix_x_v_filter*10);
		VisionData.angel_x_v=f_pix_x_v_filter+yaw_angvel;
		anglev_mix_10=VisionData.angel_x_v*10;
		VisionData.angle_x_v_filter=VisionData.angle_x_v_filter*0.25f+VisionData.angel_x_v*0.75f;
		anglev_mix_10_filter=VisionData.angle_x_v_filter*10;


//	if(abs(VisionData.tar_x-VISION_TARX)<50)
//	{
//		VisionData.angle_x_v_filter=VisionData.angel_x_v*(VisionData.tar_x-VISION_TARX)/50.0f;
//	}
//	else
//	{
//		VisionData.angle_x_v_filter=VisionData.angel_x_v;
//	}
	
//	pix_x_v_last[last_count]=pix_x_v;	//��λ���˲�����
//	last_count++;
//	last_count=last_count>FILTER_FACTOR-2?0:last_count;
}

void Tar_Move_Set(float* yaw_tarP,float dis_m,float tar_v)	//�������㣬ֻ��35��/s�ڵ�����
{
	float pre_angle_limit=80;
//	static float tar_v_fliter=0;
//	if(abs(tar_v)<2)	tar_v=0;
	if(tar_v>500)	tar_v=500;	//350
	if(tar_v<-500)	tar_v=-500;
//	tar_v_fliter=0.6f*tar_v_fliter+0.4f*tar_v;
	
	if(dis_m>5)	//�������3m����Ԥ��Ƕ�
	{
		pre_angle_limit=70-(dis_m-3.2f)*20;
		pre_angle_limit=pre_angle_limit<15?15:pre_angle_limit;
		pre_angle_limit=pre_angle_limit>70?70:pre_angle_limit;
	}
	else
	{
		pre_angle_limit=80;
	}
	
	if (dis_m>4.5f)	//2.5
	{
		dis_m=4.5f;
	}
	float shoot_delay=dis_m/Shoot_V+0.4f;	//����Ϊ��λ	//���ϳ�����ʱ0.08
	float pre_angle=tar_v*shoot_delay;
	


//	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
//	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	
	*yaw_tarP+=pre_angle;
}


u8 Auto_Shoot_Aimfdb(void)	//��׼״̬��	//
{
//	if(abs(VisionData.tar_y-VISION_TARY)<30&&abs(VisionData.tar_x-VISION_TARX)<5&&Error_Check.statu[LOST_VISION]==0&&abs(VisionData.angel_x_v)<40)
//	{
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
	if(VisionData.vision_control_state==1)
	{
		if(Auto_Shoot_AimAppraisal_Dynamic(VisionData.angel_x_v,VisionData.armor_dis,VisionData.tar_x-VISION_TARX)==1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
		
}

u8 Auto_Shoot_AimAppraisal_Static(void)	//��̬��׼��������
{
	u8 state=0;
	if(Error_Check.statu[LOST_VISION]==0)	//�з���
	{
		if(abs(VisionData.tar_y-VISION_TARY)<30&&abs(VisionData.tar_x-VISION_TARX)<5)	//��׼��
		{
			if(abs(VisionData.angel_x_v)<40)	//Ŀ���ٶȽ�С
			{
				state=1;
			}
		}
	}
	else
	{
		state=0;
	}
	
	return state;
}


/*******************************
��̬��׼����-���˲ʱ���ʱ�䴰��
1.�����������ٶȣ���Ϊ����Եģ��������Լ�Ϊ��̬��Ŀ��Ϊ��̬���Ե�ǰ�Ƕ�Ϊʵ�����
2.��������ٶȼ����루��Ч��ʱ�䣩�õ����ڵ�ǰλ�����Ŀ�����ӵ�����ʱ���������λ�ã������뵱ǰ�ǶȱȽϣ���С�ڽǶ�/λ��ĳ��ֵ������Ϊ�������
��Ч��Ԥ�⺯���ķ�����֤
ע�����ڲ�����ʱ
*******************************/
s16 test_autoshoot_error=0;	//������������ֵӦ�÷���Ϊ0
s16 test_auto_shoot_pre_angel_raw=0;
s16 test_auto_shoot_angel_error=0;

s16 Auto_Shoot_Interval_Time=750;
#define SHOOT_V_APPR	16	//����У����ٶ�
#define SHOOT_DELAY_MS 80	//�Ժ���Ϊ��λ
#define CORRECTION_FACTOR 5	//������������	//�Դ˿��ܻ���ڲв�
u8 Auto_Shoot_AimAppraisal_Dynamic(float relative_v,s16 dis_dm,s16 pix_error)	//��̬��׼��������
{
	u8 state=0;
	static u8 count=0;
	
	int delay_to_tar=dis_dm*100/SHOOT_V_APPR+CORRECTION_FACTOR;	//��ʱms
	float	angel_error=atan(pix_error/1855.2f)*57.3f;	//�Զ�Ϊ��λ
	float pre_angel_raw=delay_to_tar*relative_v/10000;	//������0.1��/sΪ��λ���ٶȳ���ms����100�����Զ�Ϊ��λ
	
	if(dis_dm>32)
	{
		if(dis_dm>41)
		{
			Auto_Shoot_Interval_Time=(s16)(1000+abs(relative_v)*4+(dis_dm-40)*80);	//����4M��Ԥ��
		}
		else
		{
			Auto_Shoot_Interval_Time=(s16)(600+abs(relative_v)*2+(dis_dm-20)*40);	//����2M
		}
	}
	else	//��3m��
	{
		if(abs(relative_v)<40)
		{
			Auto_Shoot_Interval_Time=300;
		}
		if(abs(relative_v)<60)
		{
			Auto_Shoot_Interval_Time=500;
		}
		else if(abs(relative_v)<130)
		{
			Auto_Shoot_Interval_Time=700;
		}
		else
		{
			Auto_Shoot_Interval_Time=(s16)(700+abs(relative_v)*2);
		}
	}
//	pre_angel_raw*=CORRECTION_FACTOR;
	
	if(dis_dm>40)
	{
		dis_dm=40;
	}
	
	test_auto_shoot_angel_error=(s16)(angel_error*10);
	test_auto_shoot_pre_angel_raw=(s16)(pre_angel_raw*10);
	test_autoshoot_error=(s16)((pre_angel_raw+angel_error)*10);
	if(abs(pre_angel_raw+angel_error)<Target_Range_Deal(dis_dm,0.35f))	//�Զ�Ϊ��λ	//�������Сװ������	1.6f-dis_dm*0.24f/10		0.65
	{
		count++;
		if(count>50)	count=50;
		if(count>2)	//������֡��Ч
		{
			if(abs(yunMotorData.pitch_tarP-yunMotorData.pitch_fdbP)<23&&Error_Check.statu[LOST_VISION]==0)	//δ��֡��Y��������
			{
				if(abs(VisionData.angle_x_v_filter)<410&&dis_dm<=46)	//����С��5m2���ٶ�С��41
				{
					state=1;
				}
			}
		}

	}
	else
	{
		count=0;
	}
	
	return state;
}


float Target_Range_Deal(s16 dis_dm,float armor_width)	//�Է���Ϊ��λ ������0.65f
{
	float angle_range=abs(atan(armor_width/dis_dm))*25;	//����Ӧ����57.3
	return angle_range;
}


