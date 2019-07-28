#ifndef __CONTROL_H
#define __CONTROL_H

#include "sys.h"



float MyAbs(float num);
void Control_Task(void);

void Motor_Send(void);
void Work_Execute(void);	//����ִ��2018.7.1
void Work_State_Change(void);
void Teleconltroller_Data_protect(void);	//ң���������Ա���

/***********************--����״̬--**********************/
typedef enum
{
    PREPARE_STATE,    	//�ϵ���ʼ��״̬ 1s������
    CHECK_STATE,		//�Լ�״̬ �ڴ˽׶��ܹ�ͨ���ں�������������ⷢ��ʱ�����Զ��ں�����
	CALI_STATE,    		//У׼״̬�궨
	LOST_STATE,			//Ӳ��֡�ʶ�ʧ
    NORMAL_STATE,		//��������״̬	�ֶ�״̬
	AUTO_STATE,			//�Զ�Ѳ��״̬
    STOP_STATE,      	//ֹͣ�˶�״̬(��ң�������Ƶı���״̬)
	ERROR_STATE,		//Ӳ������״̬�������������ڼ���������ʧЧ������ʧ���������޷����������
	TEST_STATE,			//�Զ���Ĳ���״̬����������״̬�򲻲��������ȵ�
}WorkState_e;

extern WorkState_e workState;
void SetWorkState(WorkState_e state);
WorkState_e GetWorkState(void);

void Reset_Task(void);
void RC_Calibration(void);	//�ϵ���ң��������ֵ����Ĭ�ϲ����Ƚϣ��ж��Ƿ�������������λ

#endif
