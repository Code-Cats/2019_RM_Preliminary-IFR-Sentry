#ifndef __MYDEF_H
#define __MYDEF_H

#include "sys.h"

#define ABS(x)	( (x>0) ? (x) : (-x) )

#define OFF 0
#define ON 1


#define WIFI_DEBUG_ENABLE 0
///********LED******/
//#define LIGHT 0

///********************************�˶�����******************************/
//#define K_CM_USUAL 10	//���̲�������ϵ��-��ͨģʽ
//#define K_PITCH 0.4f	//��������λ�ò���ϵ��
//#define K_YAW 0.005f	//ˮƽ��̨��������ϵ��


//#define SPEED_MAXDIFF 10	//task500HZ����Ƶ�ʣ�����ٶ�6000-7000��ʹ��б��ʹһ�����仯5000





/***************************FRICTION**********************************/
//#define ESC_CYCLE 14000			//2.5ms����

///***********ȡ��Ħ����************/
//#define AM_FRICTION_OPEN 1500	//�������Ų���
//#define AM_FRICTION_CLOSE 1100  
///***********���Ħ����************/
//#define LU_FRICTION_OPEN 1600	//�������Ų���
//#define LU_FRICTION_CLOSE 1100  

/**********************************************************************/












/*
WorkState_e workState = PREPARE_STATE;

static void SetWorkState(WorkState_e state)
{
    workState = state;
}


WorkState_e GetWorkState()
{
	return workState;
}

//ʹ��ʾ��
if(GetWorkState()==PREPARE_STATE) //�����׶Σ����̲���ת
	{
		ChassisSpeedRef.rotate_ref = 0;	 
	}
*/	




#endif
