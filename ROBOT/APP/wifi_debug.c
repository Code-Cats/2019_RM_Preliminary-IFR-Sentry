#include "wifi_debug.h"
#include "usart2_wifidebug_analysis.h"
#include "usart2_wifidebug.h"
#include "delay.h"
#include "yun.h"


extern u32 time_1ms_count;
extern WifiDebug_DataSend_StateTypeDef WifidebugDataSendSatet;
//const char DATA_TYPE[]= DATA_TYPE_DEF;
char Aframe_Data[FRAME_BYTES]={0};
char Apack_Data[FRAME_BYTES*FRAMENUM_IN_PACK+20+1]="#RM-DT=DATA:S0=";	//FRAME_BYTES+20+1

u16 test_FRAMENUM_IN_PACK=FRAMENUM_IN_PACK;

extern YUN_MOTOR_DATA 			yunMotorData;
extern float ZGyroModuleAngle;
extern IMU_T imu;
u32 test_countt=0;
u16 copy_index=0;
void WFDBG_DataSampling(void)	//该函数放在1ms定时器中
{
	static u32 time_record=0;
	
	if(WifidebugDataSendSatet==DebugDataSend&&time_1ms_count-time_record>=INTER_FRAME_TIME)
	{
		time_record=time_1ms_count;
		
		test_countt++;
//		Aframe_Data[0]=(char)time_1ms_count>>8;
//		Aframe_Data[1]=(char)time_1ms_count;
//		Aframe_Data[2]=(char)test_countt>>8;
//		Aframe_Data[3]=(char)test_countt;
		memcpy(&Aframe_Data[4], &imu.angleV.x, 4);
		memcpy(&Aframe_Data[4], &imu.angleV.z, 4);
		memcpy((Apack_Data+15+copy_index*FRAME_BYTES), Aframe_Data, FRAME_BYTES);
		copy_index++;
		if(copy_index>=FRAMENUM_IN_PACK)
		{
			static char sn='0';
			Apack_Data[13]=sn;
			sn++;
			if(sn>'9')	sn='0';
			memcpy((Apack_Data+15+FRAMENUM_IN_PACK*FRAME_BYTES), ";#END", 5);
			USART2_DMA_Send(Apack_Data,FRAME_BYTES*FRAMENUM_IN_PACK+20); 
			copy_index=0;
			
			copy_index=0;
		}			
	}
	
//	if(time_1ms_count%60==0&&)
//	{
//		test_countt++;
//		Aframe_Data[0]=(char)time_1ms_count>>8;
//		Aframe_Data[1]=(char)time_1ms_count;
//		Aframe_Data[2]=(char)test_countt>>8;
//		Aframe_Data[3]=(char)test_countt;
////		Aframe_Data[2]=(char)test_countt>>8;
////		Aframe_Data[3]=(char)test_countt;
////		Aframe_Data[2]=(char)test_countt>>8;
////		Aframe_Data[3]=(char)test_countt;
//		
//		//memcpy(&Aframe_Data[4], &imu.pit, 4);
//		memcpy((Apack_Data+15+copy_index*FRAME_BYTES), Aframe_Data, FRAME_BYTES);
//		copy_index++;
//		if(copy_index>=FRAMENUM_IN_PACK) copy_index=FRAMENUM_IN_PACK-1;
//	}
}
