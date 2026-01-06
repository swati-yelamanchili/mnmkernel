CC = gcc

OS_BIN = kernel.bin
OS_OBJS = kernel/boot.o kernel/kmain.o kernel/gdt.o kernel/idt.o \
          kernel/isr.o kernel/isr_s.o kernel/gdt_flush.o kernel/idt_flush.o \
          kernel/pmm.o kernel/paging.o kernel/elf_loader.o kernel/fs/initrd.o

.PHONY: all clean os qemu

all: os

os: $(OS_BIN)

$(OS_BIN): $(OS_OBJS)
	$(CC) -m32 -T kernel/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ \
	      $(shell $(CC) -m32 -print-libgcc-file-name)

kernel/boot.o: kernel/boot.S
	$(CC) -m32 -c $< -o $@

kernel/gdt_flush.o: kernel/gdt_flush.S
	$(CC) -m32 -c $< -o $@

kernel/idt_flush.o: kernel/idt_flush.S
	$(CC) -m32 -c $< -o $@

kernel/isr_s.o: kernel/isr.S
	$(CC) -m32 -c $< -o $@

kernel/kmain.o: kernel/kmain.c
	$(CC) -m32 -Ikernel -Ikernel/fs -ffreestanding -fno-stack-protector -c $< -o $@

kernel/fs/initrd.o: kernel/fs/initrd.c
	$(CC) -m32 -Ikernel -Ikernel/fs -ffreestanding -fno-stack-protector -c $< -o $@

%.o: %.c
	$(CC) -m32 -Ikernel -ffreestanding -fno-stack-protector -c $< -o $@

clean:
	rm -f $(OS_BIN) $(OS_OBJS)

qemu: $(OS_BIN)
	qemu-system-i386 -kernel $(OS_BIN) -nographic -display none -serial stdio
