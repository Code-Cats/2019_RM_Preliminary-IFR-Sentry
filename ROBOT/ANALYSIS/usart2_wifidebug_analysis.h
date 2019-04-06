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
	Unconnected,   //���ڵ�һ����Ĭ������-δ����
	Wait_connect,    //�ȴ�����������������
	Wait_OSPF1,  //���ú�͸��
	Wait_OSPF2,	//���ú�͸�������뷢��ģʽ
	Wait_HostOK,
	ConnectOK,
}WifiDebug_Connect_StateTypeDef;

typedef enum //
{
	Config_Fomat,   //���ڵ�һ����Ĭ������-δ����
	Wait_Host_INFOOK,    //�ȴ�����������������
	DebugDataSend,  //�ȴ��Լ�����͸��
}WifiDebug_DataSend_StateTypeDef;


//#RM-DT=DATA_INFO:TIM=<tim_interavl>;ABYTES=<byte_numbers>;TYPE=<s2.s2.s2.s2>;#END	//TYPE��ֵ�����ǣ�1��������+1�ֽ���.2��������+2�ֽ�������	Ŀǰֻ֧�֣�s8,u8,s16,u16,s32,u32,float



extern const char WFDBG_HEAD[];

/**    ��ѯ���ַ������ַ����е�һ�γ��ֵ�λ��    **/
s32 Get_Str_IndexOf(const char * str1, const char * str2);	//����strstr()

void WFDBG_ReplyDiscover_Send(void);

bool ESP8266_UDP_Config(char * remoteport,char * localport);	//�ú��������б�����ÿ������ms����һ��
void ESP8266_ConfigCheck(void);
void Wifi_Debug_Main(void);//������״̬��
bool ESP8266_OSPF_Config(u8 mode);	//����  OSPFģʽΪ0 �ⲿ��������ӦΪ20ms

#endif
