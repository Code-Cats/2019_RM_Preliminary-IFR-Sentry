#include "auto_aim.h"
#include "math.h"
#include "yun.h"
#include "protect.h"
#include "friction_wheel.h"
#include "usart3_judge_analysis.h"

//#include "arm_math.h"

//PITCH_GYRO_INIT 2720

extern PID_GENERAL          PID_PITCH_POSITION;
extern PID_GENERAL          PID_PITCH_SPEED;
extern PID_GENERAL          PID_YAW_POSITION;
extern PID_GENERAL          PID_YAW_SPEED;

extern YUN_MOTOR_DATA 			yunMotorData;
extern RC_Ctl_t RC_Ctl;
extern VisionDataTypeDef	VisionData;
//extern GYRO_DATA Gyro_Data;	//融合用
extern float ZGyroModuleAngle;	
extern IMU_T imu;	////融合用

float t_yaw_error=0;	//临时测试

float t_gravity_ballistic_set_angel=0;	//重力补偿角度
s16 t_gravity_ballistic_set_angel_10=0;
	
	
void aim_Pos_Recoed()
{
	
}	

	
float Pixel_to_angle(s16 pix_error)	//将像素误差转换成角度
{
	float angel_error=0;
angel_error=atan(pix_error/1460.0f)*57.3f;	//arm_atan_f32为DSP	//1855。2是去年的  视觉标定的是1750
	t_yaw_error=angel_error;
	
	angel_error=angel_error>12?12:angel_error;
	angel_error=angel_error<-12?-12:angel_error;
	//30.79
	//angel_error=pix_error/30.79f;
	
	return angel_error;
}

#define CAMERA_D	1460	//相机镜片到感光片等效像素量
float Pixel_V_to_angle_V(s16 pix_v,s16 pix_error)	//从最原始的数据进行计算可以减少单片机浮点运算的精度丢失（误差增加）
{
	
	int camera_d_2=CAMERA_D*CAMERA_D;	//距离平方
	int r_2=camera_d_2+pix_error*pix_error;	//等效半径平方
	float cos_angel_2=(float)camera_d_2/(float)r_2;
	float angel_v=0;
	angel_v=pix_v*cos_angel_2/(float)CAMERA_D;
	angel_v=angel_v*60.5f;//*2*PI;	//进行还原处理
	
	//angel_v=0.036081f*pix_v+0.1f;//+0.3026f;	//MATLAB拟合
	
	//float angel_v=0;
	return angel_v;
}

//根据距离自动设置射速
void AutoSetFrictionSpeed(u16 dis_cm)
{
	static float dis_cm_f=0;
	static u8 speed_state=0;
	
	dis_cm_f=0.6f*dis_cm_f+0.4f*dis_cm;
	
	if(dis_cm_f<330)
	{
		speed_state=0;
	}
	else if(dis_cm_f>370&&dis_cm_f<520)
	{
		speed_state=1;
	}
	else if(dis_cm_f>580)
	{
		speed_state=2;
	}
	
	switch(speed_state)
	{
		case 0:	//16m/s
		{
			frictionWheel_Data.l_wheel_tarV=16;
			if(dis_cm_f>520)
			{
				speed_state=2;
			}
			else if(dis_cm_f>370)
			{
				speed_state=1;
			}
			break;
		}
		case 1:	//21m/s
		{
			frictionWheel_Data.l_wheel_tarV=21;
			if(dis_cm_f>580)
			{
				speed_state=2;
			}
			else if(dis_cm_f<330)
			{
				speed_state=0;
			}
			break;
		}
		case 2:	//26m/s
		{
			frictionWheel_Data.l_wheel_tarV=26;
			if(dis_cm_f<370)
			{
				speed_state=0;
			}
			else if(dis_cm_f<520)
			{
				speed_state=1;
			}
			break;
		}
	}
}

//#define PITCH_INIT         7197	//2019.5.22
#define YUN_DOWN_VALUELIMIT 6500	//向下限位
#define YUN_UP_VALUELIMIT 7350	//向上限位
#define YUN_UP_DISLIMIT 120	//正常的活动范围，UP为正
#define YUN_DOWN_DISLIMIT 1120	//正常的活动范围，DOWN为负

extern FRICTIONWHEEL_DATA frictionWheel_Data;
extern ext_game_state_t game_state_judge;
// extern ext_bullet_remaining_t bullet_remaining_judge;
extern ext_game_robot_state_t robot_state_judge;

float Shoot_V=25;//18.0f;//15.5f	//14M/s
float Shoot_V_2= 625;//324.0f;//(SHOOT_V*SHOOT_V)

s32 imu_matchz_10=0;
float pix_anglev=0;
s32 pix_anglev_10=0;
s32 anglev_mix_10;
s32 anglev_mix_10_filter;

s32 imu_angelv_z_10=0;
float yaw_residual_error=0;	//打移动靶时云台跟随静差
////////////////
////////////////

s32 zgyro_int_z=0;

u16 last_tarx=0;

u8 sign_count=0;	//第三帧才开始动态识别
#define VISION_TARX 965 //963//970//990//1035//710//640+105//1053//1035是修正安装偏差1020//580	//左上原点	640
#define VISION_TARY	624//610//587//590//570//570//580//590//625//570//512+60//360//510//490//480//490//500//520//540//560//360//410//440	//左上原点	480	//打5米内目标：向上补偿518-360个像素点	//因为有阻力恒定静态误差，故补偿
void Vision_Task(float* yaw_tarP,float* pitch_tarP)	//处理目标角度
{
	if(GetWorkState()==AUTO_STATE&&robot_state_judge.mains_power_shooter_output!=0&&game_state_judge.game_progress==4)	//正常比赛模式 自动设置射速  //
	{
		AutoSetFrictionSpeed(VisionData.armor_dis);
	}
	
	
	if(frictionWheel_Data.l_wheel_tarV!=0)
	{
		Shoot_V=frictionWheel_Data.l_wheel_tarV;
		Shoot_V_2=Shoot_V*Shoot_V;
	}
	
	
	imu_angelv_z_10=(s32)(imu.angleV.z*10);
	if(Error_Check.statu[LOST_VISION]==1){	VisionData.armor_type=0;VisionData.armor_sign=0;	}//若无反馈=，该Task放在中断中主运行，及放在yun.c中以较慢频率保护运行
	//t_yaw_angel_v=Pixel_V_to_angle_V(VisionData.pix_x_v,(s16)(VisionData.error_x-VISION_TARX));
//	t_target_v=t_yaw_angel_v+Gyro_Data
	if(GetWorkState()==AUTO_STATE&&VisionData.armor_sign==1&&VisionData.armor_dis<=900)	//VisionData.armor_sign!=0
	{
		VisionData.vision_control_state=1;	//最终控制位
		PID_PITCH_SPEED.input_max=100;
		PID_PITCH_SPEED.input_min=-100;
		PID_YAW_SPEED.input_max=100;
		PID_YAW_SPEED.input_min=-100;//与自动运行函数有冲突
	}
	else
	{
		VisionData.vision_control_state=0;	//最终控制位
//		PID_PITCH_SPEED.input_max=PITCH_SPEED_PID_MAXINPUT;
//		PID_PITCH_SPEED.input_min=-PITCH_SPEED_PID_MAXINPUT;
//		PID_YAW_SPEED.input_max=YAW_SPEED_PID_MAXINPUT;
//		PID_YAW_SPEED.input_min=-YAW_SPEED_PID_MAXINPUT;
	}
	
	if(VisionData.armor_sign==1)	//保护性输出，前几帧速度置0
	{
		if(sign_count<20)
		sign_count++;
	}
	else
	{
		sign_count=0;
	}
	sign_count=10;

	VisionData.imu_vz_match= GetRecordYawAnglev(VisionData.predicttime+5);
	imu_matchz_10=(s32)(VisionData.imu_vz_match*10);
	pix_anglev=Pixel_V_to_angle_V(VisionData.pix_x_v,(s16)(VisionData.tar_x-VISION_TARX));
	
	if(sign_count>3)	//稳定后
	{
		Tar_Relative_V_Mix(VisionData.imu_vz_match,pix_anglev);	//速度融合
	}
	else
	{
		Tar_Relative_V_Mix(0,0);	//速度融合
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
		float offset_x_angle=atan(8.5f/VisionData.armor_dis)*573;
		//offset_x_angle=0;
		float zgyro_match_10=GetRecordYawAngle(VisionData.dealingtime-8)*10;
		zgyro_int_z=zgyro_match_10;
		float pitch_fdb_match=GetRecordPitchAngle(VisionData.dealingtime-5);//VisionData.dealingtime/2
		*yaw_tarP=zgyro_match_10+Pixel_to_angle((s16)(VisionData.tar_x-VISION_TARX))*10-offset_x_angle;//(float)ZGyroModuleAngle*10
		*pitch_tarP=pitch_fdb_match-Pixel_to_angle((s16)(VisionData.tar_y-VISION_TARY))*8192/360;
		t_gravity_ballistic_set_angel=Gravity_Ballistic_Set(pitch_tarP,(float)(VisionData.armor_dis_filter/100.0f));	//重力补偿
		
		//
		last_tarx=VisionData.tar_x;
		
			if(VisionData.armor_dis<400)	//只预测6m以内
			{
				Tar_Move_Set(yaw_tarP,(float)(VisionData.armor_dis/100.0f),VisionData.angle_x_v_filter);	//预测 待调节
				//Pitch_Move_Set(pitch_tarP,(float)(VisionData.armor_dis/100.0f),yunMotorData.pitch_tarV);//imu.angleV.y
			}
		
		
		//t_gravity_ballistic_set_angel_10=(s16)(t_gravity_ballistic_set_angel*10);
		
		*pitch_tarP=*pitch_tarP>(PITCH_INIT+YUN_UP_DISLIMIT)?(PITCH_INIT+YUN_UP_DISLIMIT):*pitch_tarP;	//限制行程
		*pitch_tarP=*pitch_tarP<(PITCH_INIT-YUN_DOWN_DISLIMIT)?(PITCH_INIT-YUN_DOWN_DISLIMIT):*pitch_tarP;	//限制行程
		
		yaw_residual_error=*yaw_tarP-ZGyroModuleAngle*10;	//残差记录
			
			//if(yaw_residual_error)	//自动发射
	}
	
}

#define G	9.8f	//重力加速度
float Gravity_Ballistic_Set(float* pitch_tarP,float dis_m)	//重力补偿坐标系中，向下为正
{
	if(dis_m>9)	dis_m=9;
	
	//dis_m+=0.10f;	//临时加的，因为经常打到装甲下方
//	static float tar_angle_rad_fliter=0;
	float tar_angle_rad=(PITCH_GYRO_INIT-*pitch_tarP)*0.000767f;	//弧度制简化计算2pi/8192//////////////////////////////////////////
//	tar_angle_rad_fliter=0.9f*tar_angle_rad_fliter+0.1f*tar_angle_rad;
	float sin_tar_angle=sin(tar_angle_rad);
	float gravity_ballistic_angle_rad=0;	//补偿角 弧度制
	float gravity_ballistic_angle=0;	//补偿角 角度制
	gravity_ballistic_angle_rad=0.5f*(-asin((G*dis_m*(1-sin_tar_angle*sin_tar_angle)-sin_tar_angle*Shoot_V_2)/Shoot_V_2)+tar_angle_rad);
	gravity_ballistic_angle=gravity_ballistic_angle_rad*57.3f;

	*pitch_tarP=PITCH_GYRO_INIT-gravity_ballistic_angle*8192.0f/360;//////////////////////////////

	return gravity_ballistic_angle;
}


#define PITCHANGLE_REDNUMS 80
float PitchAngleLast[PITCHANGLE_REDNUMS];
u16 PitchAngleLastcount=0;
void Record_ImuPitchAngle(float angle_z)	//记录pitch位置数据调用频率1ms POS
{
	PitchAngleLast[PitchAngleLastcount]=angle_z;
	PitchAngleLastcount++;
	if(PitchAngleLastcount>=PITCHANGLE_REDNUMS)
	{
		PitchAngleLastcount=0;
	}
}

float GetRecordPitchAngle(u16 lastcount)	//获取过去的pitch目标值 POS
{
	lastcount=lastcount>(PITCHANGLE_REDNUMS-1)?(PITCHANGLE_REDNUMS-1):lastcount;
	s16 lastindex=(s16)PitchAngleLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+PITCHANGLE_REDNUMS):lastindex;
	return PitchAngleLast[lastindex];
}


#define YAWANGLE_REDNUMS 80
float YawAngleLast[YAWANGLE_REDNUMS];
u16 YawAngleLastcount=0;
void Record_ImuYawAngle(float angle_z)	//记录yaw位置数据调用频率1ms POS
{
	YawAngleLast[YawAngleLastcount]=angle_z;
	YawAngleLastcount++;
	if(YawAngleLastcount>=YAWANGLE_REDNUMS)
	{
		YawAngleLastcount=0;
	}
}

float GetRecordYawAngle(u16 lastcount)	//获取过去的yaw目标值 POS
{
	lastcount=lastcount>(YAWANGLE_REDNUMS-1)?(YAWANGLE_REDNUMS-1):lastcount;
	s16 lastindex=(s16)YawAngleLastcount-(s16)lastcount;
	lastindex=lastindex<0?(lastindex+YAWANGLE_REDNUMS):lastindex;
	return YawAngleLast[lastindex];
}


#define YAWANGLEV_REDNUMS 100
float YawAnglevLast[YAWANGLEV_REDNUMS];
u16 YawAnglevLastcount=0;
void Record_ImuYawAnglev(float anglev_z)	//记录数据调用频率1ms
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


s16 tar_v_ronghe=0;
s16 tar_v_ronghe_filter=0;
float pix_x_v_filter=0;
#define FILTER_FACTOR	3	//5阶滤波
void Tar_Relative_V_Mix(float yaw_angvel,s16 pix_x_anglev)
{
	
	if(yaw_angvel>50)	yaw_angvel=50;	//限幅
	if(yaw_angvel<-45)	yaw_angvel=-50;
	if(pix_x_anglev>50)	pix_x_anglev=50;
	if(pix_x_anglev<-50)	pix_x_anglev=-50;
	
	pix_anglev_10=pix_x_anglev*10;
	VisionData.angel_x_v=10*(pix_x_anglev+yaw_angvel);	//融合
	VisionData.angle_x_v_filter=0.6f*VisionData.angle_x_v_filter+0.4f*VisionData.angel_x_v;
	anglev_mix_10=VisionData.angel_x_v*10;
	anglev_mix_10_filter=VisionData.angle_x_v_filter*10;
}

void Tar_Move_Set(float* yaw_tarP,float dis_m,float tar_v)	//经过计算，只打35度/s内的物体
{
	float pre_angle_limit=90;
//	static float tar_v_fliter=0;
//	if(abs(tar_v)<2)	tar_v=0;
	if(tar_v>500)	tar_v=500;	//350
	if(tar_v<-500)	tar_v=-500;
//	tar_v_fliter=0.6f*tar_v_fliter+0.4f*tar_v;
	
	//if(dis_m>5)	//距离大于3m限制预测角度
	//{
	//	pre_angle_limit=70-(dis_m-3.2f)*20;
	//	pre_angle_limit=pre_angle_limit<15?15:pre_angle_limit;
	//	pre_angle_limit=pre_angle_limit>70?70:pre_angle_limit;
	//}
	//else
	//{
	//	pre_angle_limit=80;
	//}
	
	if (dis_m>5.6)	//2.5
	{
		dis_m=5.6;
	}
	float shoot_delay=dis_m/Shoot_V*1.135f+0.07f;	//以秒为单位	//加上出弹延时0.08
	float pre_angle=tar_v*shoot_delay;
	


//	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
//	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	
	*yaw_tarP+=pre_angle;
}

void Pitch_Move_Set(float* pitch_tarP,float dis_m,float tar_v)
{
		float pre_angle_limit=40;
//	static float tar_v_fliter=0;
//	if(abs(tar_v)<2)	tar_v=0;
	if(tar_v>150)	tar_v=150;	//350
	if(tar_v<-150)	tar_v=-150;
	
	tar_v=tar_v*8192/3600;	//坐标系转换
//	tar_v_fliter=0.6f*tar_v_fliter+0.4f*tar_v;
	
	//if(dis_m>5)	//距离大于3m限制预测角度
	//{
	//	pre_angle_limit=70-(dis_m-3.2f)*20;
	//	pre_angle_limit=pre_angle_limit<15?15:pre_angle_limit;
	//	pre_angle_limit=pre_angle_limit>70?70:pre_angle_limit;
	//}
	//else
	//{
	//	pre_angle_limit=80;
	//}
	
	if (dis_m>5.6)	//2.5
	{
		dis_m=5.6;
	}
	float shoot_delay=dis_m/Shoot_V*1.1f+0.08f;	//以秒为单位	//加上出弹延时0.08
	float pre_angle=tar_v*shoot_delay;
	


//	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
//	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	if(pre_angle>pre_angle_limit)	pre_angle=pre_angle_limit;
	if(pre_angle<-pre_angle_limit)	pre_angle=-pre_angle_limit;
	
	*pitch_tarP+=pre_angle;
}

u8 Auto_Shoot_Aimfdb(void)	//瞄准状态总	//
{
//	if(ABS(VisionData.tar_y-VISION_TARY)<30&&ABS(VisionData.tar_x-VISION_TARX)<5&&Error_Check.statu[LOST_VISION]==0&&ABS(VisionData.angel_x_v)<40)
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

u8 Auto_Shoot_AimAppraisal_Static(void)	//静态瞄准评估函数
{
	u8 state=0;
	if(Error_Check.statu[LOST_VISION]==0)	//有反馈
	{
		if(ABS(VisionData.tar_y-VISION_TARY)<30&&ABS(VisionData.tar_x-VISION_TARX)<5)	//瞄准了
		{
			if(ABS(VisionData.angel_x_v)<40)	//目标速度较小
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
动态瞄准评估-获得瞬时射击时间窗口
1.输入参数相对速度，因为是相对的，所以以自己为静态，目标为动态。以当前角度为实际落点
2.根据相对速度及距离（等效出时间）得到基于当前位置射击目标在子弹到达时即将到达的位置，将其与当前角度比较，若小于角度/位置某个值，则认为可以射击
等效于预测函数的反解验证
注：存在播弹延时
*******************************/
s16 test_autoshoot_error=0;	//理想情况下这个值应该反验为0
s16 test_auto_shoot_pre_angel_raw=0;
s16 test_auto_shoot_angel_error=0;

s16 Auto_Shoot_Interval_Time=750;
#define SHOOT_V_APPR	16	//用来校验的速度
#define SHOOT_DELAY_MS 80	//以毫秒为单位
#define CORRECTION_FACTOR 5	//矫正函数修正	//以此可能会存在残差
u8 Auto_Shoot_AimAppraisal_Dynamic(float relative_v,s16 dis_dm,s16 pix_error)	//动态瞄准评估函数
{
	u8 state=0;
	static u8 count=0;
	
	int delay_to_tar=dis_dm*100/SHOOT_V_APPR+CORRECTION_FACTOR;	//延时ms
	float	angel_error=atan(pix_error/1855.2f)*57.3f;	//以度为单位
	float pre_angel_raw=delay_to_tar*relative_v/10000;	//乘以以0.1度/s为单位的速度乘以ms处以100等于以度为单位
	
	if(dis_dm>32)
	{
		if(dis_dm>41)
		{
			Auto_Shoot_Interval_Time=(s16)(1000+ABS(relative_v)*4+(dis_dm-40)*80);	//大于4M无预测
		}
		else
		{
			Auto_Shoot_Interval_Time=(s16)(600+ABS(relative_v)*2+(dis_dm-20)*40);	//大于2M
		}
	}
	else	//即3m内
	{
		if(ABS(relative_v)<40)
		{
			Auto_Shoot_Interval_Time=300;
		}
		if(ABS(relative_v)<60)
		{
			Auto_Shoot_Interval_Time=500;
		}
		else if(ABS(relative_v)<130)
		{
			Auto_Shoot_Interval_Time=700;
		}
		else
		{
			Auto_Shoot_Interval_Time=(s16)(700+ABS(relative_v)*2);
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
	if(ABS(pre_angel_raw+angel_error)<Target_Range_Deal(dis_dm,0.35f))	//以度为单位	//待加入大小装甲区分	1.6f-dis_dm*0.24f/10		0.65
	{
		count++;
		if(count>50)	count=50;
		if(count>2)	//连续三帧有效
		{
			if(ABS(yunMotorData.pitch_tarP-yunMotorData.pitch_fdbP)<23&&Error_Check.statu[LOST_VISION]==0)	//未丢帧、Y方向正常
			{
				if(ABS(VisionData.angle_x_v_filter)<410&&dis_dm<=46)	//距离小于5m2，速度小于41
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


float Target_Range_Deal(s16 dis_dm,float armor_width)	//以分米为单位 正常是0.65f
{
	float angle_range=ABS(atan(armor_width/dis_dm))*25;	//本来应该是57.3
	return angle_range;
}


