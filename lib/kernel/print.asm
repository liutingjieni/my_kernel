T1_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ(0x0003 << 3) + T1_GDT + RPL0

[bits 32]
section .text

global put_char
global set_cursor
;----------------put_char----------------
;功能描述:把栈中的1个字符写入光标所在处
;----------------------------------------
put_char:
    pushad                   ;备份32位寄存器环境
    mov ax, SELECTOR_VIDEO   
    mov gs, ax

    ;获取当前光标位置
    mov dx, 0x03d4           ;索引寄存器
    mov al, 0x0e             ;用于提供光标位置的高8位
    out dx, al
    mov dx, 0x03d5           ;通过读写数据端口0x3d5来获得或设置光标位置
    in al, dx
    mov ah, al
    
    ;获取低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx

    ;将光标存入bx
    mov bx, ax
    ;在栈中获取待打印的字符
    mov ecx, [esp + 36]      ;pushad压入4*8=32字节
                             ;加上主调函数4字节的返回地址, 故esp+36字节
    
    ;判断参数是什么字符
    cmp cl, 0xd              ;回车符CR是0x0d, 换行符LF是0x0a
    jz .is_carrisge_return 
    cmp cl, 0xa
    jz .is_line_feed

    cmp cl, 0x8              ;退格键0x8
    jz .is_backspace 
    jmp .put_other

.is_backspace:
    dec bx
    shl bx, 1                ;光标左移一位等于乘2

    mov byte [gs:bx], 0x20
    inc bx
    mov byte [gs:bx], 0x7
    shr bx, 1
    jmp set_cursor

.put_other:
    shl bx, 1

    mov [gs:bx], cl
    inc bx
    mov byte [gs:bx], 0x7
    shr bx, 1
    inc bx
    cmp bx, 2000
    jl set_cursor

.is_line_feed:
.is_carrisge_return:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx

.is_carrisge_return_end:
    add bx, 80
    cmp bx, 2000

.is_line_feed_end:
    jl set_cursor

;滚屏
.roll_screen:
    cld
    mov ecx, 960           ;一次搬4个字节, 共2000-80 = 1920
                           ;1920 * 2 / 4=960
    mov esi, 0xc00b80a0    
    mov edi, 0xc00b8000
    rep movsd

;将最后一行填充为空白
    mov ebx, 3840        
    mov ecx, 80
.cls:
    mov word[gs:ebx], 0x0720
    add ebx, 2
    loop .cls

;将光标值重置为最后一行首字符
    mov bx, 1920
set_cursor:
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

.put_char_done:
    popad
    ret


global put_str
;-----------------------------------
;put_str通过put_char来打印以0结尾的字符
;-----------------------------------
put_str:
    push ebx              ;由于
    push ecx
    xor ecx, ecx
    mov ebx, [esp + 12]
.goon:
    mov cl, [ebx]
    cmp cl, 0
    jz .str_over
    push ecx
    call put_char
    add esp, 4
    inc ebx
    jmp .goon
.str_over:
    pop ecx
    pop ebx
    ret

section .data
put_int_buffer dq 0       ;定义8字节缓冲区用于数字到字符的转换

global put_int
put_int:
    pushad
    mov ebp, esp
    mov eax, [ebp + 4 * 9]
    mov edx, eax
    mov edi, 7
    mov ecx, 8
    mov ebx, put_int_buffer

.16based_4bits:
    and edx, 0x0000000f
    cmp edx, 9
    jg .is_A2F
    add edx, '0'
    jmp .store
.is_A2F:
    sub edx, 10
    add edx, 'A'

.store:
    mov [ebx + edi], dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .16based_4bits

.ready_to_print:
    inc edi
.skip_prefix_0:
    cmp edi, 8
    je .full0

.go_on_skip:
    mov cl, [put_int_buffer + edi]
    inc edi
    cmp cl, '0'
    je .skip_prefix_0
    dec edi
    jmp .put_each_num

.full0:
    mov cl, '0'
.put_each_num:
    push ecx
    call put_char
    add esp, 4
    inc edi
    mov cl, [put_int_buffer + edi]
    cmp edi, 8
    jl .put_each_num
    popad
    ret

;-----------------------------------
;函数调用规范cdecl
;调用者将所有参数从右向左入栈
;调用者清理参数所占的栈空间
;-----------------------------------

