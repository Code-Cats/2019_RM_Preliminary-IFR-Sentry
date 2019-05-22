#include "uart7_vision_analysis.h"
#include "protect.h"
#include "auto_aim.h"
//#include "stm32f4xx.h"

extern YUN_MOTOR_DATA 			yunMotorData;

VisionDataTypeDef	VisionData={0};
VisionReceiveDataTypeDef VisionReceiveData={0};

#define DATA_LEN 12	//����֡ͷ֡β�ܳ���
u32 t_vision_count=0;
void VisionData_Receive(u8 data)	//�����崫���������ݽ�����������ͨ�ã�
{
//	LostCountFeed(&Error_Check.count[LOST_VICEBOARD]);
	if(data==0x5A&&VisionReceiveData.headOK_state==0)
	{
		VisionReceiveData.valid_state=0;	//���ݽ����ڼ䲻�������ݽ���
		VisionReceiveData.headOK_state=1;	
		VisionReceiveData.count=0;	//����count
	}
	
	if(VisionReceiveData.headOK_state==1)	//֡ͷ���ҵ�
	{
		VisionReceiveData.databuffer[VisionReceiveData.count]=data;
		VisionReceiveData.count++;
		if((data==0xA5&&VisionReceiveData.count!=DATA_LEN)||(VisionReceiveData.count>DATA_LEN))	//ʧЧ
		{
			VisionReceiveData.valid_state=0;
			VisionReceiveData.headOK_state=0;
			VisionReceiveData.count=0;	//����count
		}
		else if(data==0xA5&&VisionReceiveData.count==DATA_LEN)
		{
			VisionReceiveData.valid_state=1;
			VisionReceiveData.headOK_state=0;
			VisionReceiveData.count=0;	//����count
		}
	}
	t_vision_count++;
	//////////////////////////////��������ݽ�������-->����Ϊ��ʵ����
	if(VisionReceiveData.valid_state==1)	//����������Ч
	{
//		t_vision_count++;
		LostCountFeed(&Error_Check.count[LOST_VISION]);
		VisionData_Deal(VisionReceiveData.databuffer);
		DeviceFpsFeed(LOST_VISION);
	}
	
}


float test_pitch;
u16 tem_dis,tem_tarx,tem_tary=0;
s16 tem_speed=0;
void VisionData_Deal(volatile u8 *pData)	//�����������ڳ���֡ͷ�ĵ�1֡
{
	VisionData.armor_sign=*(pData+1)>>(7)&0x01;
	VisionData.armor_type=*(pData+1)>>(4)&0x07;
	tem_dis=*(pData+2)<<8|*(pData+3);
	tem_tarx=*(pData+4)<<8|*(pData+5);
	tem_tary=*(pData+6)<<8|*(pData+7);
	tem_speed=*(pData+8)<<8|*(pData+9);
	VisionData.dealingtime=*(pData+10);
	
	if(tem_dis<=1000)
	{
		VisionData.armor_dis=tem_dis;
		VisionData.armor_dis_filter=VisionData.armor_dis_filter*0.5f+VisionData.armor_dis*0.5f;
	}
	if(tem_tarx<2000)
	{
		VisionData.tar_x=tem_tarx;
	}
	if(tem_tary<1500)
	{
		VisionData.tar_y=tem_tary;
	}
	if(tem_speed<800)
	{
		VisionData.pix_x_v=tem_speed;
	}
	
	//if(RC_Ctl.rc.switch_right==RC_SWITCH_UP&&GetWorkState()==NORMAL_STATE )	//�����ж�������
	//{
		test_pitch=0;
		Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);	//���Ƽ�λ�������ڲ�
	//}
}
