#ifndef __BUZZER_H
#define __BUZZER_H
#include "bsp.h"

typedef struct
{
	u8 prompt_type;	//最低位表示B一下 第2位表示B两下
	
}BuzzerPromptTypeDef;

extern const u16 XinNianHao[30][3];
extern const u16 HuanLeSong[62][3];
extern const u16 LiangLiang[33+37][3];
extern const u16 Star_Sky[86][3];

#endif
