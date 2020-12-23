/*************************************************************************
	> File Name: syscall.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年12月23日 星期三 19时14分47秒
 ************************************************************************/

#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "stdint.h"
enum SYSCALL_NR {
    SYS_GETPID,
    SYS_WRITE
};
uint32_t getpid(void);
uint32_t write(char *str);
#endif
