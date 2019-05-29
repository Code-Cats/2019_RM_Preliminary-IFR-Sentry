#ifndef __AUTO_AIM_H__
#define __AUTO_AIM_H__

#include "sys.h"


#include "bsp.h"
#include "uart7_vision_analysis.h"

float Pixel_to_angle(s16 pix_error);	//将像素误差转换成角度
void Vision_Task(float* yaw_tarP,float* pitch_tarP);	//处理目标角度

float Pixel_V_to_angle_V(s16 pix_v,s16 pix_error);	//从最原始的数据进行计算可以减少单片机浮点运算的精度丢失（误差增加）

void Tar_Relative_V_Mix(float yaw_angvel,s16 pix_x_v);	//目标速度融合

float Gravity_Ballistic_Set(float* pitch_tarP,float dis_m);	//重力补偿坐标系中，向下为正

void Record_ImuPitchAngle(float angle_z);	//记录pitch位置数据调用频率1ms POS
float GetRecordPitchAngle(u16 lastcount);	//获取过去的pitch目标值 POS

void Record_ImuYawAngle(float angle_z);	//记录yaw位置数据调用频率1ms
float GetRecordYawAngle(u16 lastcount);	//获取过去的yaw目标值

void Record_ImuYawAnglev(float anglev_z);	//记录数据调用频率1ms
float GetRecordYawAnglev(u16 lastcount);

void Tar_Move_Set(float* yaw_tarP,float dis_m,float tar_v);

float Target_Range_Deal(s16 dis_dm,float armor_width);	//以分米为单位 正常是0.65f

u8 Auto_Shoot_Aimfdb(void);	//瞄准状态
u8 Auto_Shoot_AimAppraisal_Dynamic(float relative_v,s16 dis_dm,s16 pix_error);	//动态瞄准评估函数


#endif
