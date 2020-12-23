/*************************************************************************
	> File Name: syscall-init.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年12月23日 星期三 19时49分14秒
 ************************************************************************/

#ifndef _SYSCALL_INIT_H
#define _SYSCALL_INIT_H
#include "stdint.h"
#include "thread.h"
#include "print.h"
#include "syscall.h"
#include "memory.h"
uint32_t sys_getpid(void);
void syscall_init(void);
#endif
