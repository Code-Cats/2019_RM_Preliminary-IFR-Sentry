#ifndef __IMU_ANALYSIS_H
#define __IMU_ANALYSIS_H

#ifndef __IMU_H
#define __IMU_H

//#include "main.h"

#include "stm32f4xx.h"

#include "delay.h"
#include "MPU6500.h"
#include "ist8310.h"
#include "stdlib.h"

#define PI        3.1415926f
#define DEG2RAD		0.017453293f	/* 度转弧度 π/180 */
#define RAD2DEG		57.29578f		/* 弧度转度 180/π */

#define MPU_NSS_HIGH								GPIO_SetBits(GPIOF, GPIO_Pin_6)
#define MPU_NSS_LOW									GPIO_ResetBits(GPIOF, GPIO_Pin_6)

typedef	struct
{
	int16_t x;
	int16_t y;
	int16_t z;
}Axis3i16;

typedef struct
{
	Axis3i16 acceler;
	Axis3i16 magnet;

	int16_t temp;

	Axis3i16 gyro;
	
	Axis3i16 acceler_offset;
	Axis3i16 gyro_offset;	
	Axis3i16 magnet_offset;

} MPU_DATA_T;

 
typedef	struct 
{
	float x;
	float y;
	float z;
}Axis3f;

typedef	struct 
{
	float pitch;
	float roll;
	float yaw;
}EULER_ANGLR;

typedef struct
{
	Axis3f acc;

	float temp;
	float control_temp;
	u16 heat_rate;
	
	float norm_g;
	float Gravity;
	
	Axis3f angleV;		/*!< omiga, +- 2000dps => +-32768  so gx/16.384/57.3 =	rad/s */	
	
	EULER_ANGLR euler_acc;
	
	float iteration_a_pit;
	
	EULER_ANGLR euler_gyro;
	EULER_ANGLR Kalman;
	EULER_ANGLR quaternion;
	
	u8 yun_ControlMode;
} IMU_T;

void MPU_get_Data(void);

void mpu_read_bytes(u8 reg, u8 *pData, u8 len);
u8 mpu_write_byte(u8 reg, u8 data);


void MPU_Device_Init(void);
void imu_attitude_update(void);

#endif


#endif
