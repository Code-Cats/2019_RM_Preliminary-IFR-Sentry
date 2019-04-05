#ifndef __QUANTERNION_H
#define __QUANTERNION_H

#include "stm32f4xx.h"
#include "imu_analysis.h"

void imuUpdate(Axis3f acc, Axis3f gyro, float dt);


#endif
