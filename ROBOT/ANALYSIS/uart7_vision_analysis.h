#ifndef __UART5_VISION_ANALYSIS_H
#define __UART5_VISION_ANALYSIS_H

#include "stm32f4xx.h"
#include "bsp.h"

typedef struct
{
	volatile u8 headOK_state;
	volatile u8 valid_state;	//����֡��Ч��־λ
	volatile u8 databuffer[14];	//���µ��������ٶ�10λ//�ɸ����5λ��Ϊ�Ӿ�8λ����֡ͷ֡β��
	volatile u8 count;
}VisionReceiveDataTypeDef;	//�������ݴ������ݽṹ��


typedef struct
{
	volatile u8 armor_sign;	//�Ƿ�����Чװ��
	volatile u8 armor_type;	//װ������
	volatile u16 armor_dis;	//�����Ϣ
	volatile float armor_dis_filter;
	volatile u16 tar_x;	//x����
	volatile u16 tar_y;	//y����
	volatile s16 shooterror_x;	//ʵ��Ŀ��ƫ��x	��Ϊ�Զ����
	volatile s16 shooterror_y;	//ʵ��Ŀ��ƫ��y	��Ϊ�Զ����
	volatile s16 pix_x_v;
	volatile float imu_vz_match;
	volatile float angel_x_v;	//���ںϵĵ�������ٶ�	//��λΪ0.1��ÿ��
	volatile float angle_x_v_filter;
	volatile u8 predicttime;	//�����ٶȵ�ʱ��ƥ�����
	volatile u8 dealingtime;	//����λ�õ�ʱ��ƥ�����
	volatile u8 vision_control_state;	//�Ƿ��ܿ�
}VisionDataTypeDef;	//�������õ��Ĵ���������

void VisionData_Receive(u8 data);	//���Ӿ������������ݽ���У��������
void VisionData_Deal(volatile u8 *pData);	//�Ӿ�����

#endif
