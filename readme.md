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

重构自瞄文件

重构功率控制方案

加入主动重启功能  OK

先测试无额外保护区时会不会超功率

重启状态：0 1684
1 1684
2 364
3 1864

自动调整射速  OK

根据距离自动调整射速和射频

根据对战情况自动开启关闭摩擦轮  待验证BUG

待调节识别敌人受到攻击后不停止的策略

通过限制电流来限制加速度

当自身血量低于多少时就不再在打的时候静止

射速控制大致思路 以3.5m为分界线 下调变为3.3m 上跳变为3.7m 16m/s--21m/s
         5.5m分界线  5.2--5.8   21m/s--26m/s
		 射速调节放到 auto_aim中
		 
		 
赛场外屏蔽：shoot.c的射速自动保护，auto_aim的自动设置射速  control的自动运行函数

根据不同场景设置不同的反馈限幅


设置当裁判丢失时直接将shooter=1