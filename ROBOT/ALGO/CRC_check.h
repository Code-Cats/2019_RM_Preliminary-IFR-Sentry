#ifndef __CRC_CHECK_H
#define __CRC_CHECK_H

#include "stm32f4xx.h"

uint8_t Get_CRC8_Check(uint8_t *pchMessage,uint16_t dwLength);
u16 Get_CRC16_Check(uint8_t *pchMessage,uint8_t dwLength);
	
#endif
