#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

#define MAX_STROKE 2690


typedef enum	//注意控制功率限制函数  注意单位时间收到攻击血量的变化设置为切换状态的标志位
{
	CHASSIS_FINDING_ENEMY_NORMAL,	//正常情况边走边寻找目标
	CHASSIS_HIT_ENEMY_STOP,	//静止击打目标
	CHASSIS_HIT_ENEMY_NEARBY,	//寻找就近击打目标点
	CHASSIS_HIT_ENEMY_WAIST,	//以随机运动方式躲避攻击
}AUTO_OperationChassisStateTypeDef;

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

typedef struct
{
	AUTO_OperationChassisStateTypeDef chassis_state;
	AUTO_OperationYunStateTypeDef yun_state;
	u16 limitX1,limitX2;	//轨道自动运行限位
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
void Chassis_ORBITState_Run(u8 runstate);	//根据传入参数决定在轨道哪一段运行

void Auto_Operation_New(void);

void Auto_Move_Normal(void);	//普通模式全程轨道跑
void Auto_Move_Evade(void);	//闪模式随机跑
#endif
