#include "protect.h"
#include "main.h"
/*
���ļ���;���ṩ�����������㷨���������л����Լ�����ʵʱ״̬��⣬����״̬�л�
Ԥ���幦�ܣ�
1.��̨��̬��������̬����Դ�����㷨
2.�����������⼰��Ч��ֹ
3.�����ģ�����߼����
4.�����������
5.����...
����һ����¼֡�ʡ����ڵ����㣿
*/

extern RC_Ctl_t RC_Ctl;

#define LOST_THRESHOLD 10

Error_check_t Error_Check={LOST_CYCLE,{0},{0}};

u8 Error_check_workstate=1;

s16 t_error_record=0;
void LostCountAdd(u16* lostcount)	//����ı�
{
	if(*lostcount<0xFFFE)
	(*lostcount)++;
}

void LostCountFeed(u16* lostcount)	//����ı�
{
	*lostcount=0;
}

u8 LostCountCheck(u16 lostcount,u8* statu,const u16 cycle)	//����ı�
{
	if(lostcount>LOST_THRESHOLD*cycle)
		*statu=1;
	else
		*statu=0;
	return *statu;
}

void DeviceFpsFeed(u8 device_id)	//�豸FPS��¼
{
	Error_Check.fps_count[device_id]++;
}

void DeviceFpsRecoed(void)	//1s cycle
{
	for(int i=0;i<LOST_TYPE_NUM;i++)	//����ı�
	{
		Error_Check.fps[i]=Error_Check.fps_count[i];
		Error_Check.fps_count[i]=0;
	}
}

s32 test_error_Satrt=0;
void Check_Task(void)
{
	if(time_1ms_count%1000==0)
	{
		DeviceFpsRecoed();
	}
	
	for(int i=0;i<LOST_TYPE_NUM;i++)	//����ı�
	{
		LostCountAdd(&Error_Check.count[i]);
		LostCountCheck(Error_Check.count[i],&Error_Check.statu[i],Error_Check.cycle[i]);
	}
	
	
	if(Error_check_workstate==1)	//����״̬
	{
		if(Error_Check.statu[LOST_IMU]==1&&time_1ms_count>2000)
		{
			test_error_Satrt=1;
			t_error_record=LOST_IMU;
			SetWorkState(ERROR_STATE);
		}
//		
		for(int i=7;i<=9;i++)	//����ȿظ���Ҫ
		{
			if(Error_Check.statu[i]==1)	//&&i!=LOST_BULLETROTATE1
			{
				test_error_Satrt=-1;
				t_error_record=i;
				SetWorkState(ERROR_STATE);
			}
				
		}

	}
	
	
	if(Error_Check.statu[LOST_DBUS]==1)
	{
		if(GetWorkState()==CHECK_STATE||GetWorkState()==PREPARE_STATE||GetWorkState()==CALI_STATE)
		{
			SetWorkState(LOST_STATE);	//����ʱû��ң���źŵ�ѡ��
		}
		else
		{
			SetWorkState(LOST_STATE);
		}
		
	}
	
	if(RC_Ctl.key.v_h!=0||RC_Ctl.key.v_l!=0||abs(RC_Ctl.mouse.x)>3)
	{
		Error_check_workstate=0;
	}

}



