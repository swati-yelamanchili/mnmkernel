# MNMKernel — Custom 32-bit x86 OS Kernel

> A fully functional baremetal operating system written from scratch in C and x86 Assembly, capable of booting in QEMU, loading native Linux ELF binaries into Ring 3 userland, and running them under a preemptive hardware-driven scheduler.

**Authors:** Swati Yelamanchili (first author), Abhiroop (second author)

---

## What Is This?

MNMKernel is a **from-scratch x86 OS kernel** — not a hobbyist "print hello to VGA" demo, but a real operating system with hardware paging, privilege separation, ELF binary loading, syscall dispatch, and preemptive multitasking.

It boots via a custom multiboot-compliant bootloader (`boot.S`), initializes the CPU's descriptor tables (GDT/IDT), enables hardware-level memory paging through the x86 MMU, then drops into Ring 3 userland to execute standard natively-compiled Linux 32-bit ELF binaries — under a PIT-driven round-robin scheduler.

**Everything here was implemented by hand.** No libc. No external OS libraries. Pure C and x86 Assembly.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         RING 3 USERLAND                         │
│   ┌─────────────┐   ┌─────────────┐   ┌─────────────────────┐  │
│   │  hello ELF  │   │   cat ELF   │   │   (future tasks)    │  │
│   └──────┬──────┘   └──────┬──────┘   └──────────┬──────────┘  │
│          │   int $0x80 ABI │                      │             │
└──────────┼─────────────────┼──────────────────────┼────────────-┘
           │                 │                      │
┌──────────▼─────────────────▼──────────────────────▼────────────┐
│                         RING 0 KERNEL                           │
│                                                                 │
│  ┌─────────────────┐   ┌──────────────────┐                    │
│  │  Syscall Layer  │   │  IRQ Handler     │                     │
│  │  (int $0x80)    │   │  (PIC + PIT)     │                     │
│  │  read/write/    │   │  → scheduler()   │                     │
│  │  open/close/exit│   └──────────────────┘                     │
│  └────────┬────────┘                                            │
│           │                                                     │
│  ┌────────▼────────┐   ┌──────────────┐   ┌─────────────────┐  │
│  │   VFS / InitRD  │   │  ELF Loader  │   │ Task Scheduler  │  │
│  │  (ramdisk FS)   │   │  (PT_LOAD    │   │ (Round-Robin    │  │
│  │                 │   │   segments)  │   │  TCB + ESP save)│  │
│  └─────────────────┘   └──────────────┘   └─────────────────┘  │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Hardware Abstraction Layer                  │   │
│  │  GDT (Ring 0/3 segments + TSS)  │  IDT (256 entries)    │   │
│  │  PMM (physical frame allocator) │  MMU (4KB pages)      │   │
│  │  PIT (8253 timer @ 5Hz)         │  PIC (8259A cascade)  │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              boot.S (Multiboot Entry Point)              │   │
│  │           → sets up stack → calls kernel_main()         │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Features

### Bootloader & CPU Initialization
- **Multiboot-compliant `boot.S`** — sets up the stack and jumps into `kernel_main()` from real hardware
- **GDT** — 6-entry Global Descriptor Table with null, Ring 0 code/data, Ring 3 code/data, and a TSS segment for kernel stack switching on Ring 3 → Ring 0 transitions
- **IDT** — 256-entry Interrupt Descriptor Table wired to hardware exception handlers and IRQ stubs

### Memory Management
- **Physical Memory Manager (PMM)** — bitmap-based frame allocator managing available physical RAM
- **x86 MMU + Hardware Paging** — full CR3 page directory / page table setup, identity-mapping the first 4MB (kernel + VGA memory)
- **Demand Paging** — page fault handler (`#PF`, interrupt 14) dynamically allocates and maps physical frames on access; TLB invalidation via `invlpg`; newly allocated pages are zero-initialized

### Privilege Separation
- **Ring 0 / Ring 3 boundary** — the kernel runs at privilege level 0; user processes run at privilege level 3
- **TSS (Task State Segment)** — kernel stack (`esp0`) updated per-task so hardware can safely switch stacks on interrupt entry from Ring 3

### Syscall Interface
- **`int $0x80` ABI** — Linux-compatible 32-bit syscall dispatch via interrupt 128
- Implemented syscalls: `sys_write` (fd=1 → serial), `sys_open` (→ InitRD VFS), `sys_read`, `sys_close`, `sys_exit`
- Unimplemented calls return `-ENOSYS` (`-38`)

### ELF Binary Loader
- Parses standard ELF32 headers and `PT_LOAD` program headers
- Copies segments to their target virtual addresses — page faults during the copy are transparently handled by the MMU demand pager, which allocates frames on the fly
- Zero-initializes BSS regions

### Preemptive Multitasking
- **PIT (8253 Programmable Interval Timer)** — generates hardware IRQ0 at a configurable frequency
- **Round-Robin Scheduler** — saves/restores the full interrupt frame (via `esp`) for each Task Control Block (TCB), switches CR3 (page directory) per-task and updates `esp0` in the TSS
- Context switch happens entirely in kernel interrupt space — userland tasks see nothing

### InitRD & Virtual Filesystem
- **Custom ramdisk format** — bootloader loads an `initrd.img` archive; the kernel parses the header table to resolve filenames to offsets
- **File operations** — `initrd_read_file()` supports named file lookup; syscalls `open`/`read`/`close` expose these to userland
- Test binaries (`hello`, `cat`) are compiled as static 32-bit ELFs with no libc and bundled into the ramdisk

---

## Build & Run

### Requirements
- `gcc` with 32-bit support (`gcc-multilib` on Debian/Ubuntu)
- `qemu-system-i386`
- `make`

### Build

```sh
# Build the kernel binary, Ring 3 test userland binaries, and bundle the initrd
make clean && make os
make tests/bin/hello tests/bin/cat
./tools/make_initrd initrd.img tests/bin/hello tests/bin/cat
```

### Run in QEMU

```sh
qemu-system-i386 -kernel kernel.bin -initrd initrd.img -m 128M -nographic
```

Or use the Makefile shortcut:

```sh
make qemu
```

You will see serial output walking through every boot phase:

```
MNMKernel Phase 0.5 Booting [Ring 0]...
Initializing Physical Memory Manager...
Initializing GDT...
Initializing IDT...
Initializing Hardware Paging...
  Setting up Page Directory...
  Loading CR3...
  Flipping CR0 Paging Bit...
  Hardware Paging Enabled!
Found Initrd at: 0x00...
[INITRD] Initialized at ... with 2 files.
Enabling Interrupts...
Kernel Ready.
--- Phase 6: Loading Ring 3 ELFs via InitRD ---
[ELF Loader] Found valid ELF file. Entry: 0x...
[ELF Loader] PT_LOAD segment found. VAddr: 0x80000000
[MMU] Page Fault Trapped! Reaching for virtual address 0x80000000
[MMU] Granted physical frame 0x...
[MMU] Transparently Returning execution to application.
Spawning tasks and handing over to Task Scheduler...
```

---

## Project Status

| Phase | Feature | Status |
|---|---|---|
| 0 | Boot in QEMU (VGA + Serial output) | Done |
| 1 | GDT / IDT, hardware exception handling | Done |
| 2 | Physical Memory Manager | Done |
| 3 | x86 Hardware Paging + Demand Page Fault Handler | Done |
| 4 | Ring 3 Context Switch + TSS kernel stack handoff | Done |
| 5 | ELF32 binary loader (PT_LOAD parsing + segment mapping) | Done |
| 6 | Preemptive multitasking via PIT IRQ0 + round-robin scheduler | Done |
| 7 | InitRD ramdisk + VFS + `open`/`read`/`close`/`write` syscalls | Done |
| 8 | Per-process address spaces (separate CR3 per task) | Planned |
| 9 | Shell / keyboard input driver | Planned |

---

## Design Decisions & Notes

- **No cross-compiler required** — the Makefile uses host `gcc` with `-m32 -ffreestanding -nostdlib`, making it easy to build on any Linux system with `gcc-multilib` installed.
- **Demand paging is intentional** — the ELF loader copies to unmapped virtual addresses (`0x80000000+`). Each copy triggers a page fault, which the MMU handler intercepts, allocates a frame, maps it, and returns — making page allocation lazy and transparent.
- **`int $0x80` was chosen** for syscall dispatch over `sysenter`/`syscall` to stay compatible with the standard Linux 32-bit ABI, meaning real statically-compiled 32-bit ELF binaries work without modification.
- **TSS `esp0` is updated per context switch** — this ensures that when an interrupt fires while a Ring 3 task is running, the CPU uses *that task's* kernel stack, not a shared global one.

---

## Codebase Structure

```
mnmkernel/
├── kernel/
│   ├── boot.S           # Multiboot entry, stack setup
│   ├── kmain.c          # kernel_main() — boot sequence orchestration
│   ├── gdt.c / gdt.h    # GDT + TSS initialization
│   ├── idt.c / idt.h    # IDT setup, gate descriptors
│   ├── isr.S / isr.c    # ISR stubs + syscall + IRQ dispatch
│   ├── paging.c / .h    # MMU setup, page fault / demand pager
│   ├── pmm.c / .h       # Physical memory frame allocator
│   ├── elf_loader.c     # ELF32 header parser + PT_LOAD mapper
│   ├── sched.c / .h     # Round-robin scheduler, TCB management
│   ├── task.c           # Task creation helpers
│   ├── pit.c            # PIT timer initialization (IRQ0)
│   ├── linker.ld        # Kernel linker script
│   └── fs/
│       ├── initrd.c     # Ramdisk parser + file lookup
│       └── initrd.h
├── tests/
│   ├── hello32.c        # Ring 3 "hello world" (no libc, raw syscalls)
│   └── cat32.c          # Ring 3 "cat" (open/read/write via syscall)
├── tools/
│   └── make_initrd      # Utility to bundle files into initrd.img
├── Makefile
└── README.md
```

---

## What I Learned

Building this kernel required deeply understanding:
- How the **x86 privilege model** works at the hardware level (rings, segment selectors, CPL/DPL checks)
- How the **MMU translates virtual to physical addresses** via two-level page tables
- How **interrupt delivery** works from hardware → IDT → ISR → kernel C handler
- How **context switching** actually works: saving/restoring registers on the kernel stack, swapping CR3, and updating the TSS
- Why **demand paging** works: faulting on unmapped addresses, allocating frames, and transparently resuming
- How **ELF binaries are structured** and what PT_LOAD segments actually describe

---

## License

GNU Affero General Public License v3.0. See [LICENSE](LICENSE).
