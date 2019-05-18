#ifndef __HEAT_LIMIT_H
#define __HEAT_LIMIT_H

#include "bsp.h"

#define SENTRY_HEAT_MAX 480
#define SENTRY_HEAT_COOLING 160
//void Shoot_Frequency_Limit(int* ferquency,u16 rate,u16 heat);	//m/s为单位

typedef struct
{
	uint16_t bullet_num;	//发弹量	直接放在射速中断中自加
//	float bullet_speed;	//当前射速
	float heat;
	u16 maxheat;
}RobotHeatDataSimuTypeDef;	//机器热量仿真-基于射速更新


void Heat_Simulating(void);	//热量仿真//运行频率1-10HZ
void BulletNum_Simu_ADD(void);

u8 Shoot_Heat_Limit(void);	//还应当限制射频
u8 Shoot_Heat_Lost_Fre_Limit(void);	//裁判lost情况对射频的限制，反返回1是OK

#endif
