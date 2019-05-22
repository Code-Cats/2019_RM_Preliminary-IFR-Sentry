#include "uart7_vision_analysis.h"
#include "protect.h"
#include "auto_aim.h"
//#include "stm32f4xx.h"

extern YUN_MOTOR_DATA 			yunMotorData;

VisionDataTypeDef	VisionData={0};
VisionReceiveDataTypeDef VisionReceiveData={0};

#define DATA_LEN 12	//加上帧头帧尾总长度
u32 t_vision_count=0;
void VisionData_Receive(u8 data)	//从主板传过来的数据解析（主副板通用）
{
//	LostCountFeed(&Error_Check.count[LOST_VICEBOARD]);
	if(data==0x5A&&VisionReceiveData.headOK_state==0)
	{
		VisionReceiveData.valid_state=0;	//数据接受期间不进行数据解析
		VisionReceiveData.headOK_state=1;	
		VisionReceiveData.count=0;	//重置count
	}
	
	if(VisionReceiveData.headOK_state==1)	//帧头已找到
	{
		VisionReceiveData.databuffer[VisionReceiveData.count]=data;
		VisionReceiveData.count++;
		if((data==0xA5&&VisionReceiveData.count!=DATA_LEN)||(VisionReceiveData.count>DATA_LEN))	//失效
		{
			VisionReceiveData.valid_state=0;
			VisionReceiveData.headOK_state=0;
			VisionReceiveData.count=0;	//重置count
		}
		else if(data==0xA5&&VisionReceiveData.count==DATA_LEN)
		{
			VisionReceiveData.valid_state=1;
			VisionReceiveData.headOK_state=0;
			VisionReceiveData.count=0;	//重置count
		}
	}
	t_vision_count++;
	//////////////////////////////这里放数据解析函数-->解析为真实数据
	if(VisionReceiveData.valid_state==1)	//数据正常有效
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
void VisionData_Deal(volatile u8 *pData)	//传感器数据在除了帧头的第1帧
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
	
	//if(RC_Ctl.rc.switch_right==RC_SWITCH_UP&&GetWorkState()==NORMAL_STATE )	//放在中断中运行
	//{
		test_pitch=0;
		Vision_Task(&yunMotorData.yaw_tarP,&yunMotorData.pitch_tarP);	//控制键位集成再内部
	//}
}
