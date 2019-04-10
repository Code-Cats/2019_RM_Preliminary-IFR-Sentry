#include "auto_move.h"
#include "math.h"

#define CHASSIS_POS_KP 200

extern s16 ViceBoard_Position;	//0-2690	//正方向与速度一致

//u8 move_state=0;
extern s16 Chassis_Vx;
void Auto_Move_Task(s16 tar_pos,s16 speed_max)
{
	tar_pos=tar_pos<40?40:tar_pos;
	tar_pos=tar_pos>(2690-40)?(2690-40):tar_pos;
	
	if(tar_pos==ViceBoard_Position)
	{
		Chassis_Vx=(tar_pos-ViceBoard_Position)*CHASSIS_POS_KP;
	}
	else if(tar_pos>ViceBoard_Position)
	{
		Chassis_Vx=(tar_pos-ViceBoard_Position)*CHASSIS_POS_KP;
		Chassis_Vx=Chassis_Vx<50?50:Chassis_Vx;
		Chassis_Vx=Chassis_Vx>speed_max?speed_max:Chassis_Vx;
	}
	else if(tar_pos<ViceBoard_Position)
	{
		Chassis_Vx=(tar_pos-ViceBoard_Position)*CHASSIS_POS_KP;
		Chassis_Vx=Chassis_Vx>-50?-50:Chassis_Vx;
		Chassis_Vx=Chassis_Vx<-speed_max?-speed_max:Chassis_Vx;
	}
	
	
//	if(move_state==0)
//	{
//		Chassis_Vx=200;
//		if(ViceBoard_Position>2690-1000)
//		{
//			move_state=1;
//		}
//	}
//	else
//	{
//		Chassis_Vx=-200;
//		if(ViceBoard_Position<100)
//		{
//			move_state=0;
//		}
//	}
		
}


