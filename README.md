# MNMKernel — Custom 32-bit x86 OS Kernel

> A fully functional baremetal OS in C and x86 Assembly — boots in QEMU, loads Ring 3 ELF binaries, preemptive scheduler.

## What Is This?

MNMKernel is a from-scratch x86 OS kernel — no libc, no external OS libraries.
Every subsystem was implemented directly against the x86 hardware specification.

## Features

- Multiboot-compliant `boot.S` bootloader
- GDT (6 entries: null, ring0, ring3, TSS) + IDT (256 gates)
- Bitmap Physical Memory Manager
- Two-level x86 hardware paging with demand page fault handler
- Linux-compatible `int $0x80` syscall ABI (write/open/read/close/exit)
- ELF32 binary loader (PT_LOAD segments via demand pager)
- InitRD ramdisk VFS
- PIT-driven preemptive round-robin scheduler

## Build & Run

```sh
# Requirements: gcc-multilib, qemu-system-i386, make
make clean && make os
make tests/bin/hello tests/bin/cat
./tools/make_initrd initrd.img tests/bin/hello tests/bin/cat
make qemu
```

## Project Status

| Phase | Feature | Status |
|---|---|---|
| 0 | Boot in QEMU | ✅ Done |
| 1 | GDT / IDT | ✅ Done |
| 2 | PMM | ✅ Done |
| 3 | x86 Paging + Demand Pager | ✅ Done |
| 4 | Ring 3 + TSS | ✅ Done |
| 5 | ELF32 Loader | ✅ Done |
| 6 | Preemptive Scheduler | ✅ Done |
| 7 | InitRD VFS + Syscalls | ✅ Done |
| 8 | Per-process address spaces | 🔜 Planned |
| 9 | Shell / keyboard input | 🔜 Planned |

## License

MIT License. See [LICENSE](LICENSE).
