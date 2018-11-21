# bilibili_fans_monitor_595
use ESP8266 and 74HC595
鉴于不少粉丝坚持要使用74HC595驱动8位数码管，因此写了这个固件。
但595的使用方式非常灵活，能买到的模块设计也不相同，需要结合店家提供的电路图自行改写代码或自己设计电路。
该版固件为使用两个74HC595驱动8位共阳数码管设计。详细连接请看电路图。

该版本未经硬件测试，但可预见的是会有数码管闪烁问题。
原因是595不能像7219那样自动扫描，需要程序控制扫描过程。
而在循环中，获取粉丝数过程的时间是不可预测的，此时不能保证数码管的显示。
解决该问题需要使用定时器中断。
然而我当前还不知道如何在Arduino IDE中使用ESP8266的定时器中断。
如果你知道，请评论或私信告知，不胜感激。

使用7219驱动的demo
Demo: https://www.bilibili.com/video/av32002485
