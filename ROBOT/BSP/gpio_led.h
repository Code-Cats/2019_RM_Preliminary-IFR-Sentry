#ifndef __LED_H
#define __LED_H
#include "sys.h"

//LED端口定义
#define GREEN PFout(14)	// DS0
#define RED PEout(11)	// DS1	 

#define GREEN1_8(x) PGout(x)	// DSA
//#define GREEN_B PGout(2)	// DSB
//#define GREEN_C PGout(3)	// DSC
//#define GREEN_D PGout(4)	// DSD
//#define GREEN_E PGout(5)	// DSE
//#define GREEN_F PGout(6)	// DSF
//#define GREEN_G PGout(7)	// DSG
//#define GREEN_H PGout(8)	// DSH

void LED_Init(void);//初始化		 
void Green_LED_8_Init(void);	//8路路灯初始化

void Green_LED_8_Set(u8 value);
void Green_LED_8_SetNum(s8 num);
#endif
