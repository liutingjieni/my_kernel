section loader vstart = LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

total_mem_bytes dd 0

gdt_ptr dw GDT_LIMIT  ;gdt界限
        dd GDT_BASE   ;gdt起始地址


ards_buf times 244 db 0
ards_nr dw 0

loader_start:
    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buf

.e820_mem_get_loop:
    mov eax, 0x0000e820
    mov ecx, 20
    int 0x15;
    jc .e820_failed_so_try_e801
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .e820_mem_get_loop


    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx

.find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx + 8]
    add ebx, 20
    cmp edx, eax

    jge .next_ards
    mov edx, eax

.nexr_ards
    loop .find_max_mem_area
    jmp .mem_get_ok

.e820_failed_so_try_e801
    mov ax, 0xe801
    int 0x15
    jc .e801_failed_so_try88

    mov cx, 0x400
    mul cx
    shl edx, 16
    and eax, 0x0000ffff
    or edx, eax
    add edx, 0x100000
    mov esi, edx

    xor eax, eax
    mov ax, bx
    mov ecx, 0x10000
    mul ecx

    add esi, eax
    mov edx, esi
    jmp .mem_get_ok

.e801_failed_so_try88:
    mov ah, 0x88
    int 0x15
    jc .error_hlt
    and eax, 0x0000ffff

    mov cx, 0x400
    mul cx
    shl edx, 16
    or edx, eax
    add edx, 0x100000

.mem_get_ok
    mov [total_mem_bytes], edx


    






setup_page:
    mov ecx, 4096
    mov esi, 0

.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

.create_pde:
    mov eax, PAGE_DTR_TABLE_POS
    add eax, 0x1000
    mov ebx, eax
    
    or eax, PG_US_U | PG_RW_W | PG_P

    mov [PAGE_DIR_TABLE_POS + 0x0], eax
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax
    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS + 4092], eax


    mov ecx, 256
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
    mov [ebx + esi *4], edx
    add edx, 4096
    inc esi
    loop .create_pte


    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx + esi * 4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret

call setup_page
sgdt [gdt_ptr]

mov ebx, [gdt_ptr + 2]
or dword [ebx + 0x18 + 4], 0xc0000000

add dword [gdt_ptr + 2], 0xc0000000

add esp, 0xc0000000

mov eax, PAGE_DIR_TABLE_POS
mov cr3, eax

mov eax, cr0
or eax, 0x80000000
mov cr0, eax

lgdt [gdt_ptr]

mov byte [gs:160], 'V'

jmp $

