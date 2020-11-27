/*************************************************************************
	> File Name: ioqueue.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月20日 星期五 18时51分44秒
 ************************************************************************/

#ifndef _IOQUEUE_H
#define _IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"
#include "stdint.h"

#define bufsize 64

struct ioqueue {
    struct lock lock;
    struct task_struct *producer;
    struct task_struct *consumer;
    char buf[bufsize];
    int32_t head;
    int32_t tail;
};

void ioqueue_init(struct ioqueue *ioq);
int ioq_full(struct ioqueue *ioq);
int ioq_empty(struct ioqueue *ioq);
char ioq_getchar(struct ioqueue *ioq);
void ioq_putchar(struct ioqueue *ioq, char byte);
#endif
