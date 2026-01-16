# MNMKernel

A from-scratch 32-bit x86 OS kernel in C and x86 Assembly, running in QEMU.

## What Is This?

MNMKernel is a baremetal x86 OS kernel built without libc or external OS libraries.
It boots via a Multiboot-compliant bootloader, initializes the GDT and IDT for
hardware abstraction, enables virtual memory paging through the x86 MMU, and
provides a foundation for Ring 3 userland execution.

## Architecture

```
RING 0 KERNEL
  boot.S → kernel_main()
  → PMM (bitmap frame allocator)
  → GDT (6 entries: null, ring0, ring3, TSS)
  → IDT (256 gates: exceptions, IRQs, syscall)
  → MMU (page directory + demand pager)
```

## Memory Layout

| Region         | Address      |
|----------------|-------------|
| Kernel .text   | 0x00100000  |
| VGA buffer     | 0x000B8000  |
| User processes | 0x80000000+ |

## Build

Requires `gcc-multilib` and `qemu-system-i386`.

```sh
make clean && make os
make qemu
```
