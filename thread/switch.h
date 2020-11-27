/*************************************************************************
	> File Name: switch.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月14日 星期六 18时24分21秒
 ************************************************************************/

#ifndef _SWITCH_H
#define _SWITCH_H
#include "thread.h"
void switch_to(struct task_struct *cur, struct task_struct *next);
#endif
