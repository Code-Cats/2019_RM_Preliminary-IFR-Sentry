#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

#define MAX_STROKE 2690

void Auto_Operation(void);

typedef enum	//ע����ƹ������ƺ���  ע�ⵥλʱ���յ�����Ѫ���ı仯����Ϊ�л�״̬�ı�־λ
{
	CHASSIS_FINDING_ENEMY_NORMAL,	//����������߱�Ѱ��Ŀ��
	CHASSIS_HIT_ENEMY_STOP,	//ֹͣ����Ŀ��
	CHASSIS_HIT_ENEMY_NEARBY,	//�ͽ������Ŀ��
	CHASSIS_HIT_ENEMY_ELUDE,	//�������ܻ���Ŀ��
	CHASSIS_HIT_ENEMY_WAIST,	//�����Ť����ʽ����(�ܵ����蹥��)
}AUTO_OperationStateTypeDef;

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

void Auto_Operation(void);
void RobotHurtCallback(uint8_t armorid,uint8_t hurttype);

#endif
