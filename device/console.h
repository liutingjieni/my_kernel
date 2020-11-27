/*************************************************************************
	> File Name: console.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月17日 星期二 21时36分19秒
 ************************************************************************/

#ifndef _CONSOLE_H
#define _CONSOLE_H
#include "stdint.h"
void console_init(void);
void console_put_str(char *str);
void console_put_char(uint8_t char_asci);
void console_put_int(uint32_t num);
#endif
