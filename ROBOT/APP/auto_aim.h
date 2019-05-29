#ifndef __AUTO_AIM_H__
#define __AUTO_AIM_H__

#include "sys.h"


#include "bsp.h"
#include "uart7_vision_analysis.h"

float Pixel_to_angle(s16 pix_error);	//���������ת���ɽǶ�
void Vision_Task(float* yaw_tarP,float* pitch_tarP);	//����Ŀ��Ƕ�

float Pixel_V_to_angle_V(s16 pix_v,s16 pix_error);	//����ԭʼ�����ݽ��м�����Լ��ٵ�Ƭ����������ľ��ȶ�ʧ��������ӣ�

void Tar_Relative_V_Mix(float yaw_angvel,s16 pix_x_v);	//Ŀ���ٶ��ں�

float Gravity_Ballistic_Set(float* pitch_tarP,float dis_m);	//������������ϵ�У�����Ϊ��

void Record_ImuPitchAngle(float angle_z);	//��¼pitchλ�����ݵ���Ƶ��1ms POS
float GetRecordPitchAngle(u16 lastcount);	//��ȡ��ȥ��pitchĿ��ֵ POS

void Record_ImuYawAngle(float angle_z);	//��¼yawλ�����ݵ���Ƶ��1ms
float GetRecordYawAngle(u16 lastcount);	//��ȡ��ȥ��yawĿ��ֵ

void Record_ImuYawAnglev(float anglev_z);	//��¼���ݵ���Ƶ��1ms
float GetRecordYawAnglev(u16 lastcount);

void Tar_Move_Set(float* yaw_tarP,float dis_m,float tar_v);

float Target_Range_Deal(s16 dis_dm,float armor_width);	//�Է���Ϊ��λ ������0.65f

u8 Auto_Shoot_Aimfdb(void);	//��׼״̬
u8 Auto_Shoot_AimAppraisal_Dynamic(float relative_v,s16 dis_dm,s16 pix_error);	//��̬��׼��������


#endif
