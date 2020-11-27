[bits 32]
section .data
str1 db "switch"
section .text
extern put_str
global switch_to
switch_to:
    ;push str1
    ;call put_str
    ;add esp , 4
    push esi
    push edi
    push ebx
    push ebp
    mov eax, [esp + 20]
    mov [eax], esp

    mov eax, [esp + 24]
    mov esp, [eax]

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
