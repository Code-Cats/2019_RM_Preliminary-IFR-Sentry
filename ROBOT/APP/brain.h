#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

#define MAX_STROKE 2690

void Auto_Operation(void);

typedef enum
{
	NO_ENEMY,
	FIND_INFANTRY,
	FINE_HERO,
}AUTO_OperationStateTypeDef;

typedef enum
{
	FINDING_ENEMY,	//正在寻找
	FINDED_ENEMY,	//已找到或者留在保护区
}AUTO_OperationYunStateTypeDef;

typedef struct
{
	float x;
	float y;
}LastEnemyPoint2fTypeDef;

void Auto_Operation(void);

extern u8 hurt_num;

#endif
