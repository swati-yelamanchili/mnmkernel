CC = gcc
CFLAGS = -Wall -Wextra -Ikernel -Iservices -Ielf
LDFLAGS =

SIM_BIN = mnmkernel_sim
SIM_OBJS = kernel/main.o kernel/dispatcher.o kernel/uaccess.o kernel/fastpath.o services/fs/fs_service.o services/memory/mem_service.o services/process/proc_service.o elf/loader/elf_loader.o

OS_BIN = kernel.bin
OS_OBJS = kernel/boot.o kernel/kmain.o kernel/gdt.o kernel/idt.o kernel/isr.o kernel/isr_s.o kernel/gdt_flush.o kernel/idt_flush.o kernel/task.o kernel/paging.o kernel/pmm.o kernel/elf_loader.o kernel/hello32_bin.o kernel/sched.o kernel/pit.o kernel/fs/initrd.o

.PHONY: all clean test os qemu

all: sim os tests/bin/hello tests/bin/cat tests/bin/fastpath64

sim: $(SIM_BIN)

$(SIM_BIN): $(SIM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

tests/bin/hello: tests/bin/hello.c
	$(CC) -m32 -static -nostdlib $< -o $@

tests/bin/cat: tests/bin/cat.c
	$(CC) -m32 -static -nostdlib $< -o $@

tests/bin/fastpath64: tests/bin/fastpath64.c
	$(CC) -static -nostdlib -no-pie $< -o $@

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

kernel/gdt.o: kernel/gdt.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/idt.o: kernel/idt.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/isr.o: kernel/isr.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/paging.o: kernel/paging.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/pmm.o: kernel/pmm.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/elf_loader.o: kernel/elf_loader.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/task.o: kernel/task.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/hello32_bin.o: kernel/hello32_bin.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/sched.o: kernel/sched.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/pit.o: kernel/pit.c
	$(CC) -m32 -ffreestanding -fno-stack-protector -c $< -o $@

kernel/fs/initrd.o: kernel/fs/initrd.c
	$(CC) -m32 -Ikernel -Ikernel/fs -ffreestanding -fno-stack-protector -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SIM_BIN) $(OS_BIN) $(SIM_OBJS) $(OS_OBJS) tests/bin/hello tests/bin/cat

test: sim tests/bin/hello tests/bin/cat
	@echo "--- Running Unmodified Static ELF (Hello World) ---"
	./$(SIM_BIN) tests/bin/hello
	@echo "--- Running Cat execution loop ---"
	./$(SIM_BIN) tests/bin/cat

qemu: $(OS_BIN)
	qemu-system-i386 -kernel $(OS_BIN) -initrd initrd.img -nographic -display none -serial stdio
