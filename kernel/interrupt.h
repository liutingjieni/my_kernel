/*************************************************************************
	> File Name: interrupt.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月07日 星期六 20时38分48秒
 ************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H
#include "stdint.h"
#define IDT_DESC_CNT 0x30

typedef void *intr_handler;
//中断门描述符结构体
struct gate_desc;

extern intr_handler intr_entry_table[IDT_DESC_CNT];


//初始化中断描述符
static void idt_des_init(void);
//初始化可编程中断控制器
static void pic_init(void);

//完成有关中断的所有初始化工作
void idt_init(void);
static void general_intr_handler(uint8_t vec_nr);
static void exception_init(void);
void register_handler(uint8_t vector_no, intr_handler function);

//定义中断的两种状态
enum intr_status {
    INTR_OFF,            //中断关闭  
    INTR_NO             //中断打开
};

enum intr_status intr_get_status(void);
enum intr_status intr_set_status(enum intr_status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);
#endif
