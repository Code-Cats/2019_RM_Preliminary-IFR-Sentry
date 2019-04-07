#ifndef __WIFIDEBUG_H
#define __WIFIDEBUG_H
#include <stdbool.h>
#include <string.h>
#include "sys.h"

/*************用户自定义配置************
#define SENTRY1 定义兵种，共有：INFANTRY1、2  HERO1、2 ENGINEER1、2 UAV OTHER1、2、3 共12个兵种可选
#define DATA_TYPE_DEF "u2.u2.f4" "u2.u2.f4";	//要反馈的通道信息，一个通道为一个变量，用.分开每个通道；示例中"u2.u2.f4"表示通道0为u16,通道1为u16,通道2为float
#define FRAME_BYTES 8	//一帧的数据长度，即各个通道所占字节数之和，示例中为2+2+4=8 目前只支持99以内取值
#define INTER_FRAME_TIME 2	//数据的刷新（采样间隔），即相邻两帧的时间间隔,取值范围[1-9]

*************该区域为自定义区域*****************/
#define SENTRY2
#define DATA_TYPE_DEF "f4.f4.u2"
#define FRAME_BYTES 10U
#define INTER_FRAME_TIME 7U
/*************************************/



#define FRAMENUM_IN_PACK (u8)(31/INTER_FRAME_TIME)
void WFDBG_DataSampling(void);	//该函数放在1ms定时器中

#endif
