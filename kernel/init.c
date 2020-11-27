/*************************************************************************
	> File Name: kernel/init.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月05日 星期四 22时21分20秒
 ************************************************************************/
#include "init.h"
#include "print.h"
#include "timer.h"
#include "interrupt.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"

void init_all()
{
    put_str("init_all\n");
    idt_init();    //初始化中断
    mem_init();
    thread_init();
    timer_init();
    console_init();
    keyboard_init();
    tss_init();
}
