#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "bsp.h"

void Auto_Operation(void);

typedef enum
{
	NO_ENEMY,
	FIND_INFANTRY,
	FINE_HERO,
}AUTO_OperationStateTypeDef;

typedef struct
{
	float x;
	float y;
}LastEnemyPoint2fTypeDef;

void Auto_Operation(void);

#endif