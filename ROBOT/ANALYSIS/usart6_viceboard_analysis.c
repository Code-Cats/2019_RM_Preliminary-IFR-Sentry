#include "usart6_viceboard_analysis.h"
#include "protect.h"

SensorDataTypeDef SensorData={0};
//s16 ViceBoard_Position=0;
//u8 encoder_startsensor=0;
//u8 encoder_endsensor=0;
//s8 encoder_speed=0;
ViceControlDataTypeDef ViceControlData={0};
ViceBoardSendTypeDef SendData=VICEBOARD_SENDDATA_DEFAULT;
u16 t_vice_count=0;
//��������崮�ڷ��ͺ���	//2msִ��һ�Σ�10ms����һ�ν����14400�Ĳ����ʣ�һ���ֽ���ഫ��11λ��11/14400=0.76ms
//////void ViceBoard_SendDataRun(void)	
//////{
//////	if(USART_GetFlagStatus(USART6,USART_FLAG_TC)== SET)	//�����һ֡�������
//////	{
//////		if(SendData.statu==1)
//////		{
//////			SendData.data[0]=0x5A;	//��ֹ֡ͷ֡β���ƻ�
//////			SendData.data[4]=0xA5;	//��ֹ֡ͷ֡β���ƻ�
//////			USART_SendData(USART6,SendData.data[SendData.count]);
//////			t_vice_count++;
//////			SendData.count++;
//////			if(SendData.count>4)
//////			{
//////				SendData.statu=0;
//////				SendData.count=0;
//////			}
//////		}
//////	}
//////	
//////}


//////void ViceBoard_SendDataRefresh(void)//����Ƶ�ʷ��ڵ��ò�
//////{
//////	if(SendData.statu==0)	//����ѷ��͹�
//////	{
//////		SendData.data[1]=ViceControlData.valve[0]<<7|ViceControlData.valve[1]<<6|ViceControlData.valve[2]<<5|ViceControlData.valve[3]<<4|ViceControlData.valve[4]<<3|ViceControlData.valve[5]<<2|ViceControlData.servo[0]<<1|ViceControlData.servo[1];
//////		SendData.data[2]=ViceControlData.image_cut[0]<<7|ViceControlData.image_cut[1]<<6;
//////		SendData.statu=1;
//////	}
//////}

u8 test_usart8data[5];
u8 count_tt=0;
ReceiveDataTypeDef ReceiveData={0};
void ViceData_Receive(u8 data)	//�����崫���������ݽ�����������ͨ�ã�
{
	test_usart8data[count_tt]=data;
	count_tt++;
	if(count_tt>=5)
	{
		count_tt=0;
	}
	
	LostCountFeed(&Error_Check.count[LOST_VICEBOARD]);
	if(data==0x5A&&ReceiveData.headOK_state==0)
	{
		ReceiveData.valid_state=0;	//���ݽ����ڼ䲻�������ݽ���
		ReceiveData.headOK_state=1;	
		ReceiveData.count=0;	//����count
	}
	
	if(ReceiveData.headOK_state==1)	//֡ͷ���ҵ�
	{
		ReceiveData.databuffer[ReceiveData.count]=data;
		ReceiveData.count++;
		if((data==0xA5&&ReceiveData.count!=5)||(ReceiveData.count>5))	//ʧЧ
		{
			ReceiveData.valid_state=0;
			ReceiveData.headOK_state=0;
			ReceiveData.count=0;	//����count
		}
		else if(data==0xA5&&ReceiveData.count==5)
		{
			ReceiveData.valid_state=1;
			ReceiveData.headOK_state=0;
			ReceiveData.count=0;	//����count
			//////////////////////////////��������ݽ�������-->����Ϊ��ʵ����
			SensorData_Deal(ReceiveData.databuffer);
			DeviceFpsFeed(LOST_VICEBOARD);
		}
	}
	
	
}

void SensorData_Deal(volatile u8 *pData)	//�����������ڳ���֡ͷ�ĵ�1֡
{
	SensorData.encoderPos=*(pData+1);
	SensorData.encoderPos=SensorData.encoderPos<<6;
	SensorData.encoderPos+=(*(pData+2))>>2;
	SensorData.switch_start=*(pData+2)&0x01;
	SensorData.switch_end=(*(pData+2)>>1)&0x01;
	SensorData.encoderSpeed=*(pData+3);
//	ViceBoard_Position=(s16)(*(pData+1))<<8 | (s16)(*(pData+2));
//	for(int i=0;i<4;i++)
//	{
//		SensorData.Limit[i]=*(pData+1)>>(7-i)&0x01;
//	}
//	
//	for(int i=0;i<4;i++)
//	{
//		SensorData.Infrare[i]=*(pData+1)>>(3-i)&0x01;
//	}
//	
//	for(int i=0;i<4;i++)
//	{
//		SensorData.Infrare[i+4]=*(pData+2)>>(7-i)&0x01;		//[4]Ϊ���µ����ٱ�����[5]Ϊ�ϳ����	//[6]Ϊ���λ	0Ϊ����	//[7]Ϊ�Ҷ�λ	0Ϊ����
//	}
}

