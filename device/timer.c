/*************************************************************************
	> File Name: device/timer.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月09日 星期一 16时35分38秒
 ************************************************************************/

#include "timer.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "debug.h"
#include "interrupt.h"
//编程8253, 通过控制字指定使用计数器0, 工作方式2比率发生器, 并且计数器0赋予合适的计算初值
//计数器0~3的端口0~2 , 端口号0x40~0x42
//控制器寄存器的操作端口为0x43
//1193180 / 中断信号的频率 = 计算机0的初始计数值
//计数器的技术过程:
    //GATE为高电平, 即GATE为1, 这是由硬件来控制
    //计数初值已写入计数器的减法计数器, 这是由软件指令out指令控制的

//写入计数器的值, 控制频率
#define IRQ0_FREQUENCY   100
#define INPUT_FREQUENCY  1193180
#define COUNTER0_VALUE   INPUT_FREQUENCY / IRQ0_FREQUENCY
//计数器的端口号
#define COUNTER0_PORT    0x40                           //计数器的端口号

//控制器的位
#define COUNTER_NO       0                              //用来控制字中指定所使用的计数器号码, 对应与控制字的RW1和RW0
#define COUNTER_MODE     2                              //工作方式 
#define READ_WRITE_LATCH 3                              
//控制器的端口号
#define PIT_CONTROL_PORT 0x43
uint32_t ticks = 0;


static void intr_timer_handler(void)
{
    struct task_struct *cur_thread = running_thread();
//    put_int(intr_disable());
    //put_str("\n");
    ASSERT(cur_thread->stack_magic == 0x19870916);
 //   put_int(cur_thread);
    //put_int(ticks);
    //put_int(cur_thread->ticks);
    cur_thread->elapsed_ticks++;
    ticks++;

    if (cur_thread->ticks == 0) {
        //put_int(0);
      //  put_str("\n");
        cur_thread->ticks = 31;
        schedule();
    }
    else {
        cur_thread->ticks--;
    }
}

static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl, uint8_t counter_mode, uint16_t counter_value ) 
{
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

void timer_init()
{
    put_str("timer_init start\n");

    frequency_set(COUNTER0_PORT, COUNTER_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("timer_init end");
}

