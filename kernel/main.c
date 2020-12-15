/*************************************************************************
	> File Name: main.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月02日 星期一 22时32分55秒
 ************************************************************************/
#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "thread.h"
#include "list.h"
#include "console.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "process.h"
void k_thread_a(void *);
void k_thread_b(void *);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0;
int test_var_b = 0;


int main(void)
{
    put_str("l am kernel\n");
    init_all();
    put_str("l am kernel\n");
    //ASSERT(1==2);
    //thread_start("k_thread_a", 31, k_thread_a, "argA ");
    //thread_start("k_thread_b", 8, k_thread_b, "argB ");
    //put_int(u_prog_a);
    process_execute(u_prog_a, "user_prog_a");
    process_execute(u_prog_b, "user_prog_b");
    intr_enable();
    
    while(1);
    //while(1)
    //{
    //   // put_str("Main ");
    //    console_put_str("Main ");
    //}
    return 0;
}

void k_thread_a(void *arg)
{
    while (1) {
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kdb_buf)) {
            //console_put_str(arg);
            char byte = ioq_getchar(&kdb_buf);
            console_put_char(byte);
        }
        //put_str(para);
        intr_set_status(old_status);
    }
}

void k_thread_b(void *arg)
{
    while (1) {
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kdb_buf)) {
            //console_put_str(arg);
            char byte = ioq_getchar(&kdb_buf);
            console_put_char(byte);
        }
        //put_str(para);
        intr_set_status(old_status);
    }
    char *para = arg;
}

void u_prog_a(void)
{
    while(1) {
        put_str("lala\n");
        test_var_a++;
    }
}

void u_prog_b(void)
{
    while(1) {
        put_str("lblb\n");
        test_var_b++;
    }
}
