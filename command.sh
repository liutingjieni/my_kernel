#!/bin/bash 
nasm -f elf -o build/print.o lib/kernel/print.asm && 
nasm -f elf -o build/kernel.o kernel/kernel.asm && 
nasm -f elf -o build/switch.o thread/switch.asm && 
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/  -I userprog/ -c -fno-builtin -o build/tss.o userprog/tss.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/  -I userprog/ -I device/ -c -fno-builtin -o build/process.o userprog/process.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/ -c -fno-builtin -o build/console.o device/console.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/ -c -fno-builtin -o build/keyboard.o device/keyboard.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/ -c -fno-builtin -o build/ioqueue.o device/ioqueue.c -m32 &&
gcc -I lib/kernel/ -I lib/  -I device/ -I thread/ -I userprog/ -c -fno-builtin -o build/init.o kernel/init.c -m32 &&
gcc -I lib/kernel/ -I lib/  -c -fno-builtin -o build/interrupt.o kernel/interrupt.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/  -c -fno-builtin -o build/list.o lib/kernel/list.c -m32 &&
gcc -I lib/kernel/ -I lib/  -I thread/ -I kernel/ -c -fno-builtin -o build/timer.o device/timer.c -m32 &&
gcc -I lib/kernel/ -I lib/  -I thread/ -c -fno-builtin -o build/memory.o kernel/memory.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/  -c -fno-builtin -o build/bitmap.o lib/kernel/bitmap.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/  -c -fno-builtin -o build/string.o lib/string.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/  -I userprog/ -c -fno-builtin -o build/thread.o thread/thread.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I kernel/ -I thread/  -c -fno-builtin -o build/sync.o thread/sync.c -m32 &&
gcc -I lib/kernel/ -I lib/ -I thread/ -I device/ -I userprog/ -c -fno-builtin -o build/main.o kernel/main.c -m32 &&
gcc -I kernel/ -I lib/kernel/ -I lib/  -I device/ -c -fno-builtin -o build/debug.o kernel/debug.c -m32 &&
ld -Ttext 0xc0001500 -e main -o build/kernel.bin -m elf_i386  build/main.o build/print.o build/kernel.o build/init.o build/interrupt.o build/timer.o build/thread.o build/debug.o build/memory.o build/bitmap.o  build/string.o build/switch.o build/list.o build/console.o build/sync.o build/keyboard.o build/ioqueue.o build/tss.o build/process.o&& 
dd if=build/kernel.bin of=~/c.img bs=512 count=200 seek=9 conv=notrunc

