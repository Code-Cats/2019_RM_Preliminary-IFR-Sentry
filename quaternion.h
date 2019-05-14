#ifndef __QUANTERNION_H
#define __QUANTERNION_H

#include "stm32f4xx.h"
#include "imu_analysis.h"

typedef	struct 
{
	float x;
	float y;
	float z;
}Axis3f;

void imuUpdate(Axis3f acc, Axis3f gyro, float dt);


#endif
