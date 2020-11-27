/*************************************************************************
	> File Name: memory.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月11日 星期三 14时59分15秒
 ************************************************************************/

#ifndef _MEMORY_H
#define _MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "debug.h"
#include "string.h"

struct virtual_addr {
    struct bitmap vaddr_bitmap;     //虚拟地址用到的位图结构
    uint32_t vaddr_start;           //虚拟地址起始地址
};

extern struct pool kernel_pool, user_pool;

void mem_init(void);

//内存池标记, 用于判断用哪个内存池
enum pool_flags {
    PF_KERNEL = 1,                 //内盒内存池
    PF_USER = 2                  //用户内存池
};


#define PG_P_1 1                   //页目录项或页表项存在属性位
#define PG_P_0 0                   
#define PG_RW_R 0                  //R/W属性位值
#define PG_RW_W 2
#define PG_US_S 0                  //系统位
#define PG_US_U 4                  //用户位

void *get_kernel_pages(uint32_t);

uint32_t addr_v2p(uint32_t vaddr);
void *get_a_page(enum pool_flags pf, uint32_t vaddr);
#endif
