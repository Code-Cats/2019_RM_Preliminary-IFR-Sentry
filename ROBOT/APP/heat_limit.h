#ifndef __HEAT_LIMIT_H
#define __HEAT_LIMIT_H

#include "bsp.h"

#define SENTRY_HEAT_MAX 480
#define SENTRY_HEAT_COOLING 160
//void Shoot_Frequency_Limit(int* ferquency,u16 rate,u16 heat);	//m/sΪ��λ

typedef struct
{
	uint16_t bullet_num;	//������	ֱ�ӷ��������ж����Լ�
//	float bullet_speed;	//��ǰ����
	float heat;
	u16 maxheat;
}RobotHeatDataSimuTypeDef;	//������������-�������ٸ���


void Heat_Simulating(void);	//��������//����Ƶ��1-10HZ
void BulletNum_Simu_ADD(void);

u8 Shoot_Heat_Limit(void);	//��Ӧ��������Ƶ
u8 Shoot_Heat_Lost_Fre_Limit(void);	//����lost�������Ƶ�����ƣ�������1��OK

#endif
