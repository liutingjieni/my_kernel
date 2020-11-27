/*************************************************************************
	> File Name: process.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月25日 星期三 14时56分52秒
 ************************************************************************/

#include "process.h"
#include "global.h"
#include "interrupt.h"
#include "memory.h"
#include "console.h"

extern void intr_exit(void);

//初始化用户进程上下文信息
void start_process(void *filename_)
{
    console_put_str("start_proess start\n");
    void *function = filename_;
    //put_int(function);
    struct task_struct *cur = running_thread();
    cur->self_kstack += sizeof(struct thread_stack);
    struct intr_stack *proc_stack = (struct intr_stack *)cur->self_kstack;
    proc_stack->edi= proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;

    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;           //操作系统不允许用户进程访问显存， 所以将其初始化为0
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function;
    proc_stack->cs = SELECTOR_U_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    //在4GB的虚拟地址空间中，(0xc0000000 - 1)是用户空间的最高地址，
    //USER_STACK3_VADDR = 0xc0000000 - 0x1000, 是用户栈空间的下边界 + PG_SIZE是栈的上边界， 即栈底
    proc_stack->esp = (void *)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
    proc_stack->ss = SELECTOR_U_DATA;
    console_put_str("start_proess start\n");
    asm volatile("movl %0, %%esp; jmp intr_exit": : "g"(proc_stack): "memory");
    console_put_str("start_proess end\n");
}

//激活页表
void page_dir_activate(struct task_struct *p_thread)
{
    uint32_t pagedir_phy_addr = 0x100000;
    if (p_thread->pgdir != NULL) {
        pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);
    }
    asm volatile("movl %0, %%cr3" : : "r"(pagedir_phy_addr) : "memory");
}

//激活进程或线程的页表， 更新tss中的esp0为进程的特权级0的栈
void process_activate(struct task_struct *p_thread)
{
    ASSERT(p_thread != NULL);
    page_dir_activate(p_thread);
    if (p_thread->pgdir) {
        update_tss_esp(p_thread);
    }
}

//创建页目录表
uint32_t *create_page_dir(void)
{
    //分配物理地址， 虚拟地址， 并完成映射
    uint32_t *page_dir_vaddr = get_kernel_pages(1);
    if (page_dir_vaddr == NULL) {
        console_put_str("create_page_dir: get_kernel_page failed!");
        return NULL;
    }

    //复制页表, 内核的页表
    memcpy((uint32_t *)((uint32_t)page_dir_vaddr + 0x300 * 4), (uint32_t *)(0xfffff000 + 0x300 * 4), 1024);

    //得到物理地址
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
    //将页目录表的最高4字节处填入
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U |PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

//创建用户进程虚拟地址位图
void create_user_vaddr_bitmap(struct task_struct *user_prog)
{
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);
    user_prog->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

//创建用户进程, filename是用户进程地址, name是进程名
void process_execute(void *filename, char *name)
{
    console_put_str("process_execute start\n");
    struct task_struct *thread = get_kernel_pages(1);
    init_thread(thread, name, default_prio);
    create_user_vaddr_bitmap(thread);
    thread_create(thread, start_process, filename);
    thread->pgdir = create_page_dir();

    enum intr_status old_status = intr_disable();
    ASSERT(!elem_find(&thread_ready_list,  &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    intr_set_status(old_status);

    console_put_str("process_execute end\n");
}
