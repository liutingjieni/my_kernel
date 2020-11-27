/*************************************************************************
	> File Name: kernel/interrupt.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月05日 星期四 16时56分52秒
 ************************************************************************/
#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"


#define PIC_M_CTRL  0x20                //主片的控制端口0x20
#define PIC_M_DATA 0x21                 //主片的数据端口0x21
#define PIC_S_CTRL 0xa0                 //从片的控制端口0xa0
#define PIC_S_DATA 0xal                 //从片的数据端口0xa1

#define EFLAGS_IF 0x00000200
//pushfl是将eflags寄存器的值压栈, 然后再用popl指令将其弹出到EFLAG_VAR关闭的约束中
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0;" : "=g"(EFLAG_VAR))

//中断门描述符结构体
struct gate_desc {
    uint16_t func_offset_low_word;      //中断处理程序在目标代码段偏移量的偏移量高16位
    uint16_t selector;                  //中断处理程序目标代码段选择子
    uint8_t dcount;                     //双字计数字段
    uint8_t attribute;                  //属性位
    uint16_t func_offset_high_word;      //中断处理程序在目标代码段偏移量的偏移量低16位
};

char *intr_name[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];
static struct gate_desc idt[IDT_DESC_CNT];


//创建中断门描述符
static void make_idt_desc(struct gate_desc*p_gdesc, uint8_t attr, intr_handler function)
{
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;
}

//初始化中断描述符
static void idt_des_init(void)
{
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    put_str("   idt_des_init done\n");
    put_int(IDT_DESC_ATTR_DPL0);
}

//初始化可编程中断控制器
//
static void pic_init(void)
{
    outb(PIC_M_CTRL, 0x11);     
    outb(PIC_M_DATA, 0x20);      //ICW2: 起始中断向量号0x20
                                 //IR[0~7]为0x20~0x27
    outb(PIC_M_DATA, 0x04);      //ICW3: IR2接从片
    outb(PIC_M_DATA, 0x01);      //ICW4: 8086模式, 正常EOI
    
    outb(PIC_S_CTRL, 0x11);      //ICW1: 边沿触发. 级联8259, 需要ICW4
    outb(PIC_S_DATA, 0x28);      //ICW2: 起始中断向量号0x28
                                 //也就是IR[8~15]为0x28~0x2F
    outb(PIC_S_DATA, 0x02);      //ICW3: 设置从片连接到主片的IR2引脚
    outb(PIC_S_DATA, 0x01);      //ICW4: 8086模式, 正常EOI

    //打开主片上的IR0, 也就是目前只接受时钟产生的中断
    outb(PIC_M_DATA, 0xfc);   // 0xf8, 0xbf-> 0xfd, 0xff只打开时钟中断
    outb(PIC_S_DATA, 0xff);

    put_str("   pic_init done\n");
}

static void general_intr_handler(uint8_t vec_nr)
{
    if (vec_nr == 0x27 || vec_nr == 0x2f) {
        return;
    }

    //put_str("!!!!!!!! exction message begin !!!!!!!!!!!");
    put_int(vec_nr);
   // set_cursor(0);
   // int cursor_pos = 0;
   // while (cursor_pos < 320) {
   //     put_char(' ');
   //     cursor_pos++;
   // }

   // set_cursor(0);
   // put_str("!!!!!!!! exction message begin !!!!!!!!!!!\n");
   // set_cursor(88);
   // put_str(intr_name[vec_nr]);
   // if (vec_nr == 14) {
   //     int page_fault_vaddr = 0;
   //     asm ("movl %%cr2, %0" : "=r"(page_fault_vaddr));

   //     put_str("\n page fault addr is ");
   //     put_int(page_fault_vaddr);
   // }
   // put_str("\n!!!!!!!! exction message end !!xx");
    //while(1);
}

static void exception_init(void)
{
    put_str("exception_init\n");
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        idt_table[i] = general_intr_handler;

        intr_name[i] = "unknown";
    }
    intr_name[0] = "#DE Divide Error";
   intr_name[1] = "#DB Debug Exception";
   intr_name[2] = "NMI Interrupt";
   intr_name[3] = "#BP Breakpoint Exception";
   intr_name[4] = "#OF Overflow Exception";
   intr_name[5] = "#BR BOUND Range Exceeded Exception";
   intr_name[6] = "#UD Invalid Opcode Exception";
   intr_name[7] = "#NM Device Not Available Exception";
   intr_name[8] = "#DF Double Fault Exception";
   intr_name[9] = "Coprocessor Segment Overrun";
   intr_name[10] = "#TS Invalid TSS Exception";
   intr_name[11] = "#NP Segment Not Present";
   intr_name[12] = "#SS Stack Fault Exception";
   intr_name[13] = "#GP General Protection Exception";
   intr_name[14] = "#PF Page-Fault Exception";
   // intr_name[15] 第15项是intel保留项，未使用
   intr_name[16] = "#MF x87 FPU Floating-Point Error";
   intr_name[17] = "#AC Alignment Check Exception";
   intr_name[18] = "#MC Machine-Check Exception";
   intr_name[19] = "#XF SIMD Floating-Point Exception";
}

//完成有关中断的所有初始化工作
void idt_init()
{
    idt_des_init();              //初始化中断描述符表
    exception_init();
    pic_init();                  //初始化8259A

    //加载idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
    asm volatile("lidt %0": : "m"(idt_operand));
    put_str("idt_init done\n");
}


//开中断并返回花开中断前的状态
enum intr_status intr_enable(void)
{
    enum intr_status old_status;
    if (INTR_NO == intr_get_status()) {
        old_status = INTR_NO;
        return old_status;
    } else {
        old_status = INTR_OFF;
        asm volatile("sti");
        return old_status;
    }
}

//关中断, 并且返回关中断前的状态
enum intr_status intr_disable(void)
{
    enum intr_status old_status;
    if (INTR_NO == intr_get_status()) {
        old_status = INTR_NO;
        asm volatile("cli": : :"memory");
        return old_status;
    } else {
        old_status = INTR_OFF;
        return old_status;
    }
}

//将当前状态设置为status
enum intr_status intr_set_status(enum intr_status status)
{
    return status & INTR_NO ? intr_enable() : intr_disable();
}

//获取当前中断状态
enum intr_status intr_get_status()
{
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_NO : INTR_OFF;
}

void register_handler(uint8_t vector_no, intr_handler function)
{
    idt_table[vector_no] = function;
}
