/*************************************************************************
	> File Name: process.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月25日 星期三 22时40分48秒
 ************************************************************************/

#ifndef _PROCESS_H
#define _PROCESS_H
#include "tss.h"

#define USER_VADDR_START 0x804800
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define default_prio 310
void process_activate(struct task_struct *p_thread);
void process_execute(void *filename, char *name);
#endif
