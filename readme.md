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

