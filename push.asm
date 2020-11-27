%include "boot.inc"

section push32_test vstart=0x900
jmp loader_start

gdt_addr:
    GDT_BASE: dd 0x00000000
              dd 0x00000000

    CODE_BASE: dd 0x0000ffff
               dd DESC_CODE_HIGH4

    DATA_STACK_DESC: dd 0x0000ffff
                     dd DESC_DATA_HIGH4

    VIDEO_DESC: dd 0x80000008
                dd DESC_VIDEO_HIGH4

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
    SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

    gdt_ptr: dw GDT_LIMIT
             dd gdt_addr

    loader_start:

    ;打开A20
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ;加载GDT
    lgdt [gdt_ptr]

    ;cr0第0位置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    ;刷新流水线, 避免分支预测的影响, 这种CPU优化策略, 最怕jmp跳转
    ;这将导致之前做的预测失效, 从而起到了刷新的作用

[bit 32]
p_mod_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x900
    push byte 0x7
    push word 0x8
    push dword 0x900
    jmp $
