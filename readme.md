# IFR2019-Progra_standard-F427_template

bsp:首先led gpio各口初始化
延时100ms 等待模块稳定
各个模块初始化
CAN初始化
定时器初始化

定时器在pre模式等待各模块OK了再进入CHECK模式
再自检 如果有错误就重启
自检通过

USART:
1 DBUS
2 蓝牙
3
6
7
8


调试模式与赛场模式屏蔽记录：
shoot.c : AUTO_STATE close LASER
brain.c : CHASSIS_AUTORUN
		  YUN_AUTORUN
		  AUTO_SHOOT


待更改REMOTE DMA空闲触发


目标buffer设置斜坡

在3s使用完后 使tarbuffer=judge_buffer?
根据裁判数据回传自动开关摩擦轮和强制进入自动模式