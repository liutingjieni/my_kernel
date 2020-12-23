/*************************************************************************
	> File Name: syscall-init.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年12月23日 星期三 19时04分35秒
 ************************************************************************/

#include "syscall-init.h"

#define syscall_nr 32
typedef void *syscall;
syscall syscall_table[syscall_nr];

uint32_t sys_getpid(void)
{
    return running_thread()->pid;
}

void syscall_init(void)
{
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    put_str("syscall_init done\n");
}
