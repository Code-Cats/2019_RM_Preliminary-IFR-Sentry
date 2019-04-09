#include "auto_move.h"
#include "math.h"

extern s16 ViceBoard_Position;	//0-2690

u8 move_state=0;
extern s16 Chassis_Vx;
void Auto_Move_Task(void)
{
	if(move_state==0)
	{
		Chassis_Vx=200;
		if(ViceBoard_Position>2690-1000)
		{
			move_state=1;
		}
	}
	else
	{
		Chassis_Vx=-200;
		if(ViceBoard_Position<100)
		{
			move_state=0;
		}
	}
		
}


