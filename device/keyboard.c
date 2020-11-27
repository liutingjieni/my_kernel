/*************************************************************************
	> File Name: keyboard.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月19日 星期四 22时16分48秒
 ************************************************************************/

#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "ioqueue.h"
#define KBD_BUF_PORT 0x60 //键盘buffer寄存器端口号为0x60

struct ioqueue kdb_buf;

#define esc       '\033'
#define backspace '\b'
#define tab       '\t'
#define enter     '\r'
#define delete    '\177'

#define char_invisible 0
#define ctrl_l_char    char_invisible
#define ctrl_r_char    char_invisible
#define shift_l_char   char_invisible
#define shift_r_char   char_invisible
#define alt_l_char     char_invisible
#define alt_r_char     char_invisible
#define caps_lock_char char_invisible

#define shift_l_make 0x2a
#define shift_r_make 0x36
#define alt_l_make   0x38
#define alt_r_make   0xe038
#define alt_r_break  0xe0b8
#define ctrl_l_make  0x1d
#define ctrl_r_make  0xe01d
#define ctrl_r_break 0xe09d
#define caps_lock_make 0x3a


static int ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

static char keymap[][2] = {
    {0, 0},
    {esc, esc},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {backspace, backspace},
    {tab, tab},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {enter, enter},
    {ctrl_l_char, ctrl_l_char},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {shift_l_char, shift_l_char},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {shift_r_char, shift_r_char},
    {'*', '*'},
    {alt_l_char, alt_l_char},
    {' ', ' '},
    {caps_lock_char, caps_lock_char}
};

//键盘中断处理函数
static void intr_keyboard_handler(void)
{
    //uint8_t scancode = inb(KBD_BUF_PORT);
    //put_int(scancode);
    //return;
    
    //这次中断发生前的上一次中断, 以下三个按键是否有按下
    int ctrl_down_last = ctrl_status;
    int shift_down_last = shift_status;
    int caps_lock_last = caps_lock_status;

    int break_code;
    //从8042的输出缓冲器读出数据
    uint16_t scancode = inb(KBD_BUF_PORT);

    //若扫描码scancode是e0开头的, 表示此键的按下将产生多个扫描码
    //所以马上结束此次中断处理函数, 等待下一个扫描码进来
    if (scancode == 0xe0) {
        ext_scancode = 1;
        return;
    }
    
    //如果上次的扫描码是以0xe0开头的,将扫描码合并
    if (ext_scancode) {
        scancode = ((0xe000) | scancode);
        ext_scancode = 0;
    }
    //判断是否是断码. 断码 = 通码 + 0x80;
    break_code = ((scancode & 0x0080) != 0);    

    //断码
    if (break_code) {
        uint16_t make_code = (scancode &= 0xff7f);
        if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
            ctrl_status = 0;
        } 
        else if (make_code == shift_l_make || make_code == shift_r_make){
            shift_status = 0;
        }
        else if (make_code == alt_l_make || make_code == alt_r_make) {
            alt_status = 0;
        }
        return;
    }
    //通码
    else if ((scancode > 0x00 && scancode < 0x3b) || (scancode == alt_r_make) || (scancode == ctrl_r_make)) {
        int shift = 0;
        if (scancode < 0x0e || scancode == 0x29 ||
             scancode == 0x1a || scancode == 0x1b ||
             scancode == 0x2b || scancode == 0x27 ||
             scancode == 0x28 || scancode == 0x33 ||
             scancode == 0x34 || scancode == 0x35 ) {
            if (shift_down_last) {
                shift = 1;
            }
        }
        else {
            if (shift_down_last && caps_lock_last) {
                shift = 0;
            }
            else if (shift_down_last || caps_lock_last) {
                shift = 1;
            }
            else {
                shift = 0;
            }
        }
        uint8_t index = (scancode &= 0x00ff);

        char cur_char = keymap[index][shift];
        if (cur_char) {
            if (!ioq_full(&kdb_buf)) {
                //put_char(cur_char);
                ioq_putchar(&kdb_buf, cur_char);
            }
            return;
        }
        if (scancode == ctrl_l_make || scancode == ctrl_r_make) {
            ctrl_status = 1;
        }
        else if (scancode == shift_l_make || scancode == shift_r_make) {
            shift_status = 1;
        }
        else if(scancode == caps_lock_make) {
            caps_lock_status = !caps_lock_status;
        }
    }
    else {
        put_str("unknow key!\n");
    }
}

void keyboard_init()
{
    put_str("keyboard init start\n");
    ioqueue_init(&kdb_buf);
    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard_init done\n");
}
