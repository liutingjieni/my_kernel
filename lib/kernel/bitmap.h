/*************************************************************************
	> File Name: bitmap.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月11日 星期三 13时29分43秒
 ************************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

#include "stdint.h"
#define BITMAP_MASK 1
struct bitmap {
    uint32_t btmp_bytes_len;
    uint8_t *bits;
};

void bitmap_init(struct bitmap *btmp);
int bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx);
int bitmap_scan(struct bitmap *btmp, uint32_t cnt);
void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value);
#endif
