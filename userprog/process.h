/*************************************************************************
	> File Name: process.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月25日 星期三 22时40分48秒
 ************************************************************************/

#ifndef _PROCESS_H
#define _PROCESS_H
#include "tss.h"

//Linux用户进程的起始地址
#define USER_VADDR_START 0x8048000
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define default_prio 31
void process_activate(struct task_struct *p_thread);
void process_execute(void *filename, char *name);
#endif
