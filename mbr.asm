%include "boot.inc"

section MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 184fh

    int 10h

    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0x70

    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0x70

    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xA4
    
    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0x70
    
    mov byte [gs:0x08], 'R'
    mov byte [gs:0x09], 0xA4

    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR;
    mov cx, 4
    call rd_disk_m_16

    jmp LOADER_BASE_ADDR

rd_disk_m_16: 
                        ;eax=LBA扇区号
                        ;bx=将数据写入的内存地址
                        ;cx=读入的扇区数
    mov esi, eax ;
    mov di, cx

    mov dx, 0x1f2
    mov al,cl
    out dx, al

    mov eax, esi

    ;0x173, 0x174, 0x175表示LBA模式的0~23位 
    mov dx, 0x1f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f         ;0x1f6端口0~3位表示LBA模式的24~27位
    or al, 0xe0          ;第5,7位为1, 第4位dev(主盘或从盘), 第6位MOD位(寻址模式LBA:!, CHS:0)
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7        ;0x1f7端口写操作时commmand寄存器,0x20,即读硬盘
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx            ;同一端口,写时表示写入命令字,读时表示读入硬盘状态
    and al, 0x88         ;第三位为1表示硬盘控制器已准备好数据传输
                         ;第7位为1表示硬盘忙
    cmp al, 0x08
    jnz .not_ready

;第五步:从0x1f0端口读数据
    mov ax, di
    mov dx, 256
    mul dx         ;dx:ax<-dx*ax
    mov cx, ax     
;di为要读取的扇区数,一个扇区有512字节,每次读入一个字
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret

times 510-($-$$) db 0
db 0x55, 0xaa
