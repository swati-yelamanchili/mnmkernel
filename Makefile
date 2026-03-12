CC = gcc
CFLAGS = -Wall -Wextra -Ikernel -Iservices -Ielf

OS_BIN = kernel.bin
OS_OBJS = kernel/boot.o kernel/kmain.o kernel/gdt.o kernel/idt.o \
          kernel/isr.o kernel/isr_s.o kernel/gdt_flush.o kernel/idt_flush.o \
          kernel/task.o kernel/paging.o kernel/pmm.o kernel/elf_loader.o \
          kernel/hello32_bin.o kernel/sched.o kernel/pit.o kernel/fs/initrd.o

.PHONY: all clean os qemu

all: os tests/bin/hello tests/bin/cat

os: $(OS_BIN)

$(OS_BIN): $(OS_OBJS)
	$(CC) -m32 -T kernel/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ $(shell $(CC) $(CFLAGS) -m32 -print-libgcc-file-name)

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
tests/bin/hello: tests/bin/hello.c
	$(CC) -m32 -static -nostdlib $< -o $@
tests/bin/cat: tests/bin/cat.c
	$(CC) -m32 -static -nostdlib $< -o $@
%.o: %.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

clean:
	rm -f $(OS_BIN) $(OS_OBJS) tests/bin/hello tests/bin/cat
qemu: $(OS_BIN)
	qemu-system-i386 -kernel $(OS_BIN) -initrd initrd.img -nographic -display none -serial stdio
