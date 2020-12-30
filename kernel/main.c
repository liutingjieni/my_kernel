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
#include "syscall.h"
#include "syscall-init.h"
#include "stdio.h"
void k_thread_a(void *);
void k_thread_b(void *);
void u_prog_a(void);
void u_prog_b(void);
//int test_var_a = 0;
//int test_var_b = 0;

int prog_a_pid = 0, prog_b_pid = 0;

int main(void)
{
    put_str("l am kernel\n");
    init_all();
    //ASSERT(1==2);
    //put_int(u_prog_a);
    //process_execute(u_prog_a, "user_prog_a");
    //process_execute(u_prog_b, "user_prog_b");
    intr_enable();
    //console_put_str("main_pid: 0x");
    //console_put_int(sys_getpid());
    //console_put_char('\n');
    
    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 31, k_thread_b, "argB ");
    
    while(1);
    //while(1)
    //{
    //   // put_str("Main ");
    //    console_put_str("Main ");
    //}
    return 0;
}

void k_thread_a(void* arg) {     
   char* para = arg;
   //void *addr = sys_malloc(33);
   console_put_str("I am thread_a, sys_malloc(33), addr is 0x");
   console_put_str("thread_a end\n");
   //console_put_str("thread_a_pid: 0x");
   //console_put_int(sys_getpid());
   //console_put_char('\n');
   //console_put_str("prog_a_pid: 0x");
   //console_put_int(prog_a_pid);
   //console_put_char('\n');
   while(1);
   //while(1) {
   //   console_put_str(" v_a:0x");
   //   console_put_int(test_var_a);
   //   console_put_str("\n");
   //}
}

void k_thread_b(void* arg) {     
   char* para = arg;
   //void *addr = sys_malloc(63);
   console_put_str("I am thread_b, sys_malloc(63), addr is 0x");
   console_put_str("thread_b end\n");

   //console_put_str("prog_a_pid: 0x");
   //console_put_int(prog_a_pid);
   //console_put_char('\n');
   while(1);
   //char* para = arg;
   //while(1) {
   //   console_put_str(" v_b:0x");
   //   console_put_int(test_var_b);
   //   console_put_str("\n");
   //}
}

/*
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

*/

void u_prog_a(void) {
   //prog_a_pid = getpid();
   char *name = "prog_a";
   printf(" I am %s, my pid: %d%c", name, getpid(), '\n');
   // printf("prog_a_pid:0x%x\n", getpid());
    while(1);
    //while(1) {
   //   //console_put_str("lalalalala ");
   //   test_var_a++;
   //}
}

void u_prog_b(void) {
   char *name = "prog_a";
   printf(" I am %s, my pid: %d%c", name, getpid(), '\n');
   // printf("prog_b_pid:0x%x\n", getpid());
   //prog_a_pid = getpid();
   while(1);
   //while(1) {
   //   //console_put_str("lblblblb ");
   //   test_var_b++;
   //}
}


