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

### 操作系统内核的实现
1. 计算机启动过程
a. 我们按下电源开始, CS:IP置位, 指向0xffff0的位置, 这个地址是在ROM固化的BIOS程序, 紧接着就是BIOS的POST(Power on Self Test, 上电自检)过程了, BIOS对计算机各个部件开始初始化, 如果有错误会给出警告. 当BIOS完成这些工作之后, 它的工作就是在外部存储设备中寻找操作系统, 安装过操作系统的同学都知道, BIOS中有一张启动设备表, BIOS会按照这个表列出的顺序查找可启动设备. 那么怎么该设备是否是可以启动的呢? 如果这个存储设备的第一个扇区512个字节的最后两个字节是0x55和0xAA, 那么该设备是可启动设备. 这是一个约定, 所以BIOS会对这个列表中的设备逐一检测, 只要有一个设备满足要求, 后续的设备将不再测试.所以我们将mbr.bin加载到硬盘的第一个扇区, bochs会将mbr.bin加载到0x7c00这个位置, 而一个扇区只有512字节, 放不下很多内容, 所以我们有了loader.bin. mbr负责将loader加载到指定物理地址(0x900)
b. loader.asm负责获取物理内存总量(total_mem_bytes, 物理地址0xb00)
   loader.asm负责进入保护模式,包括构建全局描述符表, 打开A20, 加载GDT, cr0寄存器第0位置1, 然后刷新流水线
   loader.asm还负责采用分页机制, 包括准备好页目录表及页表, 将页表地址写入控制寄存器, 寄存器cr0的PG位置1.
   loader.asm还负责加载接下来的内核文件/build/kernel.bin, kernel.bin的位置可以由我们自定义, ld的-Ttext可以指定我们的入口地址(我们是0xc0015000)
2. 
2. 中断定时器的实现
- 中断定义: 由于CPU获知了计算机中发生的某些事, CPU暂停正在执行的程序, 转而去执行处理该事件的程序, 当这段程序执行完毕后, CPU继续执行刚才的程序. 整个过程称为中断处理, 也称中断.
- 为什么要有中断? 
  因为有了中断, 系统才能并发运行. **没有中断, 操作系统几乎什么都做不了, 操作系统是中断驱动的**
- 中断分类
  - 外部中断按是否宕机来划分, 可分为可屏蔽中断和不可屏蔽中断
    外部中断是指来自外部的中断, 而外部的中断源必须是某个硬件, 所以外部中断又称为硬件中断. CPU提供统一的接口作为中断信号的公共线路, 所有来自外设的中断信号的共享公共线路连接到CPU. CPU为外部中断提供了两根信号线INTR(INTeRrupt)和NMI(Non Maskable Interrupt), INTR上是可屏蔽中断, NMI上是不可屏蔽中断.
  - 内部中断按中断是否正常, 可分为软中断和异常
  软中断就是软件主动发起的异常
- 添加中断描述符表
  中断描述符表(Interrupt Descriptor Table, IDT)是保护模式下用于存储中断处理程序入口的表, 当CPU接受到一个中断时, 需要用中断向量在此表中索引对应的描述符, 在该描述符中找到中断处理程序的起始地址, 然后执行中断处理程序. 中断门描述符表可以包括中断门描述符表, 任务门描述符表, 调用门描述符表, 陷阱门描述符表(详见https://blog.csdn.net/qq_43811102/article/details/109487557), my_kernel中只使用了中断门描述符.
-中断处理过程及保护
  完整的中断过程分为CPU内和CPU外两部分:
  CPU外: 外部设备的中断由中断代理芯片接收, 处理后该中断的中断向量号发送到CPU
  CPU内: CPU执行该中断向量号对应的中断处理程序
  这里我们只讨论处理器内的过程, 处理器外的过程属于硬件范畴, 我们暂不讨论
  1. 处理器根据中断向量号定位中断门描述符
  2. 处理器进行特权级检查
    a. 如果是软中断int n, int3和into引发的中断, 这些是用户进程主动发起的中断, 由用户代码控制, 处理器要检查当前特权级CPL和门描述符DPL, 这是检查进门的特权下限, 如果CPL权限大于等于目标代码段DPL, 即数值上CPL<=门描述符DPL, 特权级"门槛"检查通过, 进入下一步的"门框"检查. 否则, 处理器抛出异常
    b. 这一步检查特权级的上限: 处理器要检查当前特权级CPL和门描述符中所记录的选择子对应的目标代码段DPL, 如果CPL权限小于目标代码段DPL, 即数值上CPL>目标代码段DPL, 检查通过. 否则CPL若大于等于目标代码段DPL, 处理器将引发异常, 也就是说, 除了用返回指令从高特权级返回,特权转移只能发生在由低转高
    若中断是由外部设备和异常引起的, 只直接检查CPL和目标代码段的DPL, 和上面的步骤是一样的, 要求CPL权限小于目标代码段DPL, 即数值上CPL>目标代码段DPL, 否则处理器引发异常
  3. 执行中断处理程序
  特权级检查通过后, 将门描述符目标代码段选择子加载到代码段寄存器CS中, 把门描述符中中断处理程序的偏移地址加载到EIP, 开始执行中断处理程序.
  初始化中断描述符表, 初始化中断处理函数

- 完成中断请求和定时器中断

  
  
