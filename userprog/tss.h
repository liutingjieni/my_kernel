/*************************************************************************
	> File Name: tss.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月23日 星期一 17时53分58秒
 ************************************************************************/

#ifndef _TSS_H
#define _TSS_H
#include "../thread/thread.h"
void tss_init(void);
void update_tss_esp(struct task_struct *pthread);
#endif
