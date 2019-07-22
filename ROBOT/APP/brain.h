#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

#define MAX_STROKE 2690


typedef enum	//注意控制功率限制函数  注意单位时间收到攻击血量的变化设置为切换状态的标志位
{
	CHASSIS_FINDING_ENEMY_NORMAL,	//正常情况边走边寻找目标
	CHASSIS_HIT_ENEMY_STOP,	//停止击打目标
	CHASSIS_HIT_ENEMY_NEARBY,	//就近点击打目标
	CHASSIS_HIT_ENEMY_ELUDE,	//优先闪避击打目标
	CHASSIS_HIT_ENEMY_WAIST,	//以随机扭腰方式闪避(受到大弹丸攻击)
}AUTO_OperationStateTypeDef;

typedef enum
{
	YUN_FINDING_ENEMY,	//正在寻找
	YUN_FINDED_ENEMY,	//已找到或者留在保护区
}AUTO_OperationYunStateTypeDef;

typedef struct
{
	float x;
	float y;
}LastEnemyPoint2fTypeDef;

void Auto_Operation(void);
void RobotHurtCallback(uint8_t armorid,uint8_t hurttype);

u8 Recognize_EncoderPosState(s16 pos);
void Chassis_ORBITState_Run(u8 runstate);	//根据传入参数决定在轨道哪一段运行

void Auto_Move_NewTest(void);	//结合动能转化的轨道优化策略测试
#endif
