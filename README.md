# my_kernel

项目概述:
1. 介绍
本项目的出发点在于, 在学习操作系统课程感受到各种机制的抽象, 没有办法去将这门课程落到实处, 尽管在学习过程中, 老师提供了大量的实验, 去让我们了解操作系统, 但总有一种管中窥豹的感觉, 于是就产生了一个想法, 自己是否可以开发一个小的操作系统内核, 来解决内心的疑问, 于是就有了这个项目, 本项目是基于Intel x86架构及其兼容计算机上构建一个简单的操作系统内核, 与具有商业价值的操作系统内核相较而言依旧相差甚远, 但通过实践, 可以充分理解x86保护模式的运行方式和操作系统的基本原理. 我认为这些是通过阅读书籍所不能获得的.
2. 开发环境
  - gcc, C语言编译器
  - ld, 链接器
  - nasm, 开源免费的汇编编译器
  - bochs, 开源的虚拟机, 支持调试, 安装方法https://blog.csdn.net/qq_43811102/article/details/107760227
  - dd, 
3. 安装方法:
  git clone
  cd my_kernel
  dd 
  ./command
 
- 本项目基于x86-32位开发, 实现了内存管理, 线程调度, 进程管理
操作系统内核的实现
1. 计算机启动过程
我们按下电源开始, CS:IP置位, 指向0xffff0的位置, 这个地址是在ROM固化的BIOS程序, 紧接着就是BIOS的POST(Power on Self Test, 上电自检)过程了, BIOS对计算机各个部件开始初始化, 如果有错误会给出警告. 当BIOS完成这些工作之后, 它的工作就是在外部存储设备中寻找操作系统, 安装过操作系统的同学都知道, BIOS中有一张启动设备表, BIOS会按照这个表列出的顺序查找可启动设备. 那么怎么该设备是否是可以启动的呢? 如果这个存储设备的第一个扇区512个字节的最后两个字节是0x55和0xAA, 那么该设备是可启动设备. 这是一个约定, 所以BIOS会对这个列表中的设备逐一检测, 只要有一个设备满足要求, 后续的设备将不再测试.所以我们将mbr.bin加载到硬盘的第一个扇区, bochs会将mbr.bin加载到0x7c00这个位置, 而一个扇区只有512字节, 放不下很多内容, 所以我们有了loader.bin. 首先loader.bin写入我们指定的磁盘位置(我们是0x2),由mbr.bin加载指定物理地址(我们是0x900), loader.asm负责获取物理内存总量(total_mem_bytes, 物理地址0xb00)loader.asm负责进入保护模式,包括构建全局描述符表, 打开A20, 加载GDT, cr0寄存器第0位置1, 然后刷新流水线, loader.asm还负责采用分页机制, 包括准备好页目录表及页表, 将页表地址写入控制寄存器, 寄存器cr0的PG位置1. loader.asm还负责加载接下来的内核文件/build/kernel.bin, kernel.bin的位置可以由我们自定义, ld的-Ttext可以指定我们的入口地址(我们是0xc0015000)
编写了MBR主引导记录, MBR是由硬盘的第一个启动引导扇区, 里面的程序即bootloader, bios自动执行机器自检, 建立好中断向量表后, 将mbr上的程序加载0x7c00: 0x0000的位置, bootloader的主要功能是加载内核的引导文件, 在我们的系统李为loader.asm
2. 

