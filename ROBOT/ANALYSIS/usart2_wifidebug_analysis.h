#ifndef __USART2_WIFIDEBUG_ANALYSIS_H
#define __USART2_WIFIDEBUG_ANALYSIS_H
#include <stdbool.h>
#include <string.h>
#include "sys.h"

#if FRAME_BYTES>99 
this is a error
#endif

#if INTER_FRAME_TIME>9
this is a error
#endif

typedef enum
{
	Wait_PowerOn,
	Wait_ConnectWIFI,
	Wait_GetLocalIP,
	Wait_MonitorUDP,
	ESP8266_OK,
}ESP8266_Config_StateTypeDef;

typedef enum //
{
	Unconnected,   //放在第一个是默认类型-未连接
	Wait_connect,    //等待主机发起握手连接
	Wait_OSPF1,  //配置好透传
	Wait_OSPF2,	//配置好透传并进入发送模式
	Wait_HostOK,
	ConnectOK,
}WifiDebug_Connect_StateTypeDef;

typedef enum //
{
	Config_Fomat,   //放在第一个是默认类型-未连接
	Wait_Host_INFOOK,    //等待主机发起握手连接
	DebugDataSend,  //等待自己进入透传
}WifiDebug_DataSend_StateTypeDef;


//#RM-DT=DATA_INFO:TIM=<tim_interavl>;ABYTES=<byte_numbers>;TYPE=<s2.s2.s2.s2>;#END	//TYPE的值含义是：1数据类型+1字节数.2数据类型+2字节数……	目前只支持：s8,u8,s16,u16,s32,u32,float



extern const char WFDBG_HEAD[];

/**    查询子字符串在字符串中第一次出现的位置    **/
s32 Get_Str_IndexOf(const char * str1, const char * str2);	//基于strstr()

void WFDBG_ReplyDiscover_Send(void);

bool ESP8266_UDP_Config(char * remoteport,char * localport);	//该函数的运行背景是每隔若干ms运行一次
void ESP8266_ConfigCheck(void);
void Wifi_Debug_Main(void);//主运行状态机
bool ESP8266_OSPF_Config(u8 mode);	//设置  OSPF模式为0 外部调用周期应为20ms

#endif
