/*************************************************************************
	> File Name: print.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月03日 星期二 18时31分15秒
 ************************************************************************/

#ifndef _PRINT_H
#define _PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char *message);
void put_int(uint32_t num);
void set_cursor(uint32_t);
#endif
