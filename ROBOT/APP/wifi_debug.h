#ifndef __WIFIDEBUG_H
#define __WIFIDEBUG_H
#include <stdbool.h>
#include <string.h>
#include "sys.h"

/*************�û��Զ�������************
#define SENTRY1 ������֣����У�INFANTRY1��2  HERO1��2 ENGINEER1��2 UAV OTHER1��2��3 ��12�����ֿ�ѡ
#define DATA_TYPE_DEF "u2.u2.f4" "u2.u2.f4";	//Ҫ������ͨ����Ϣ��һ��ͨ��Ϊһ����������.�ֿ�ÿ��ͨ����ʾ����"u2.u2.f4"��ʾͨ��0Ϊu16,ͨ��1Ϊu16,ͨ��2Ϊfloat
#define FRAME_BYTES 8	//һ֡�����ݳ��ȣ�������ͨ����ռ�ֽ���֮�ͣ�ʾ����Ϊ2+2+4=8 Ŀǰֻ֧��99����ȡֵ
#define INTER_FRAME_TIME 2	//���ݵ�ˢ�£��������������������֡��ʱ����,ȡֵ��Χ[1-9]

*************������Ϊ�Զ�������*****************/
#define SENTRY2
#define DATA_TYPE_DEF "f4.f4.u2"
#define FRAME_BYTES 10U
#define INTER_FRAME_TIME 7U
/*************************************/



#define FRAMENUM_IN_PACK (u8)(31/INTER_FRAME_TIME)
void WFDBG_DataSampling(void);	//�ú�������1ms��ʱ����

#endif
