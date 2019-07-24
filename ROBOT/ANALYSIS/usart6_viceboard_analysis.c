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
//主板给副板串口发送函数	//2ms执行一次，10ms更新一次结果，14400的波特率，一个字节最多传输11位，11/14400=0.76ms
//////void ViceBoard_SendDataRun(void)	
//////{
//////	if(USART_GetFlagStatus(USART6,USART_FLAG_TC)== SET)	//如果上一帧发送完成
//////	{
//////		if(SendData.statu==1)
//////		{
//////			SendData.data[0]=0x5A;	//防止帧头帧尾被破坏
//////			SendData.data[4]=0xA5;	//防止帧头帧尾被破坏
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


//////void ViceBoard_SendDataRefresh(void)//限制频率放在调用层
//////{
//////	if(SendData.statu==0)	//如果已发送过
//////	{
//////		SendData.data[1]=ViceControlData.valve[0]<<7|ViceControlData.valve[1]<<6|ViceControlData.valve[2]<<5|ViceControlData.valve[3]<<4|ViceControlData.valve[4]<<3|ViceControlData.valve[5]<<2|ViceControlData.servo[0]<<1|ViceControlData.servo[1];
//////		SendData.data[2]=ViceControlData.image_cut[0]<<7|ViceControlData.image_cut[1]<<6;
//////		SendData.statu=1;
//////	}
//////}

u8 test_usart8data[5];
u8 count_tt=0;
ReceiveDataTypeDef ReceiveData={0};
void ViceData_Receive(u8 data)	//从主板传过来的数据解析（主副板通用）
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
		ReceiveData.valid_state=0;	//数据接受期间不进行数据解析
		ReceiveData.headOK_state=1;	
		ReceiveData.count=0;	//重置count
	}
	
	if(ReceiveData.headOK_state==1)	//帧头已找到
	{
		ReceiveData.databuffer[ReceiveData.count]=data;
		ReceiveData.count++;
		if((data==0xA5&&ReceiveData.count!=5)||(ReceiveData.count>5))	//失效
		{
			ReceiveData.valid_state=0;
			ReceiveData.headOK_state=0;
			ReceiveData.count=0;	//重置count
		}
		else if(data==0xA5&&ReceiveData.count==5)
		{
			ReceiveData.valid_state=1;
			ReceiveData.headOK_state=0;
			ReceiveData.count=0;	//重置count
			//////////////////////////////这里放数据解析函数-->解析为真实数据
			SensorData_Deal(ReceiveData.databuffer);
			DeviceFpsFeed(LOST_VICEBOARD);
		}
	}
	
	
}

void SensorData_Deal(volatile u8 *pData)	//传感器数据在除了帧头的第1帧
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
//		SensorData.Infrare[i+4]=*(pData+2)>>(7-i)&0x01;		//[4]为上下岛加速保护，[5]为拖车检测	//[6]为左对位	0为触发	//[7]为右对位	0为触发
//	}
}

