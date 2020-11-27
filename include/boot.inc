;-----------------loader和kernel-----------------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

;低32位: 0~15 段界限 16~32 段基址
;高32位: 0~7  段基址 8~11type  12S 13~14DPL 15 P
;        16~19段界限 20AVL 21L 22D/B 23G 24~31段基址 

;符号名一律采用"DSEC_字段名_字段相关信息"
;-----------------gdt描述符属性------------------
DESC_G_4k equ 1_00000000000000000000000b ;3byte 粒度位

;有效位数
DESC_D_32 equ  1_0000000000000000000000b ;23bit 

;设置是否是64位代码段
DESC_L equ   0_000000000000000000000b ;22bit

;保留位
DESC_AVL equ  0_00000000000000000000b 

DESC_LIMIT_CODE2 equ  1111_0000000000000000b 
DESC_LIMIT_DATA2 equ  DESC_LIMIT_CODE2 
DESC_LIMIT_VIDEO2 equ 0000_0000000000000000b 

;present存在位, 段是否存在内存当中
DESC_P equ  1_000000000000000b 

;DPL(Descriptor Privilege Level)描述符特权级
DESC_DPL_0 equ  00_0000000000000b 
DESC_DPL_1 equ  01_0000000000000b 
DESC_DPL_2 equ  10_0000000000000b 
DESC_DPL_3 equ  11_0000000000000b 

;S位指示是否是系统段, 在CPU眼里分为两大类, 要么描述的是系统段,要么是数据段
;凡是硬件运行需要用到的东西都可称之为系统段
;凡是软件(操作系统也属于软件, CPU眼里吗他与用户程序无区别)需要的东西都称之为数据
;s=0 为系统段
DESC_S_CODE equ  1_000000000000b 
DESC_S_DATA equ  DESC_S_CODE
DESC_S_sys equ   0_000000000000b 

;用于表示内存段或门的子类型
;x=1,c=0,r=0,a=0 代码段是可执行的,非一致性,不可读,已访问位a清0
DESC_TYPE_CODE equ  1000_00000000b  
;x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写,已访问a清0
DESC_TYPE_DATA equ 0010_00000000b

DESC_BASE_VIDEO2 equ 1011b
;定义了代码段的高4字节
;(0x00 << 24)表示"段基址24~31"字段, 后面是加法表达式, 描述段描述符高四字节的内容
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4k + DESC_D_32 + \
DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
DESC_P + DESC_DPL_0 + DESC_S_CODE + \
DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ (0x00 << 24)  + DESC_G_4k + \
DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00


DESC_VIDEO_HIGH4 equ (0x00 << 24)+ DESC_BASE_VIDEO2  + \
DESC_G_4k + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
DESC_P + DESC_DPL_0 + DESC_S_DATA + \
DESC_TYPE_DATA + 0x00

;定义选择子的属性
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

TI_GDT equ 000b
TI_LDT equ 100b

;kernel采用平坦模型

PAGE_DIR_TABLE_POS equ 0x100000

PG_P equ 1b       ;present存在位,该页是否在物理内存中
PG_RW_R equ 00b   ;读写位
PG_RW_W equ 10b 
PG_US_S equ 000b  ;user/supervisor普通用户/超级用户位
PG_US_U equ 100b

KERNEL_START_SECTOR equ 0x9            ;kernel.bin的硬盘所在位置
KERNEL_BIN_BASE_ADDR equ 0x70000      ;elf被加载的位置
KERNEL_ENTRY_POINT equ 0xc0001500    ;分析后被重新加载的位置
