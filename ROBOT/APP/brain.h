#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

#define MAX_STROKE 2690


typedef enum	//ע����ƹ������ƺ���  ע�ⵥλʱ���յ�����Ѫ���ı仯����Ϊ�л�״̬�ı�־λ
{
	CHASSIS_FINDING_ENEMY_NORMAL,	//����������߱�Ѱ��Ŀ��
	CHASSIS_HIT_ENEMY_STOP,	//��ֹ����Ŀ��
	CHASSIS_HIT_ENEMY_NEARBY,	//Ѱ�Ҿͽ�����Ŀ���
	CHASSIS_HIT_ENEMY_WAIST,	//������˶���ʽ��ܹ���
}AUTO_OperationChassisStateTypeDef;

typedef enum
{
	YUN_FINDING_ENEMY,	//����Ѱ��
	YUN_FINDED_ENEMY,	//���ҵ��������ڱ�����
}AUTO_OperationYunStateTypeDef;

typedef struct
{
	float x;
	float y;
}LastEnemyPoint2fTypeDef;

typedef struct
{
	AUTO_OperationChassisStateTypeDef chassis_state;
	AUTO_OperationYunStateTypeDef yun_state;
	u16 limitX1,limitX2;	//����Զ�������λ
	u16 tar_chassispos;
	u8 chassis_enable;
	u8 tar_remainbuffer;
	u8 real_remainbuffer;
	u8 last_dir;
	u32 robot_hits_count;
	u32 yun_lost_count;
}AutoOperationDataTypeDef;

void Auto_Operation(void);
void RobotHurtCallback(uint8_t armorid,uint8_t hurttype);

u8 Recognize_EncoderPosState(s16 pos);
void Chassis_ORBITState_Run(u8 runstate);	//���ݴ�����������ڹ����һ������

void Auto_Operation_New(void);

void Auto_Move_Normal(void);	//��ͨģʽȫ�̹����
void Auto_Move_Evade(void);	//��ģʽ�����
#endif
