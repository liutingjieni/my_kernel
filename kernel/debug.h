/*************************************************************************
	> File Name: debug.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月10日 星期二 19时46分31秒
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H


void panic_spin(char *filename, int line, const char* func, const char* condition);
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) if(CONDITION) {} else {PANIC(#CONDITION); }
#endif //NDEBUG


#endif //__DEBUG_H
