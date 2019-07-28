#ifndef __PROTECT_H
#define __PROTECT_H
#include "stm32f4xx.h"
#include "control.h"

enum LOST_TYPES
{
	LOST_IMU1,
	LOST_IMU2,
	LOST_DBUS,
	LOST_VICEBOARD,
	LOST_REFEREE,
	LOST_VISION,
	LOST_CM1,
	LOST_CM2,
	LOST_YAW,
	LOST_PITCH,
	LOST_SM,
	LOST_FM_RIGHT,
	LOST_FM_LEFT,
	LOST_TYPE_NUM,
};


typedef struct
{
	const u16 cycle[LOST_TYPE_NUM];
	u16 count[LOST_TYPE_NUM];
	u8 statu[LOST_TYPE_NUM];
	u16 fps[LOST_TYPE_NUM];
	u16 fps_count[LOST_TYPE_NUM];
}Error_check_t;

extern Error_check_t Error_Check;

#define LOST_CYCLE \
{\
	4,\
	4,\
	17,\
	4,\
	10,\
	20,\
	4,\
	4,\
	4,\
	4,\
	4,\
	4,\
	4,\
}

void LostCountAdd(u16* lostcount);
void LostCountFeed(u16* lostcoun);
	u8 LostCountCheck(u16 lostcoun,u8* statu,const u16 cycle);

void DeviceFpsFeed(u8 device_id);	//Éè±¸FPS¼ÇÂ¼

void Check_Task(void);	//control.c


#endif
