#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef struct tcb {
    uint32_t id;
    uint32_t esp;       // Kernel stack pointer when preempted
    uint32_t cr3;       // Page directory physical address
    uint32_t state;     // 0 = empty, 1 = running, 2 = ready
    uint32_t kernel_stack; // Top of the kernel stack for this task
} tcb_t;

void sched_init(void);
uint32_t schedule(uint32_t esp);
void add_task(uint32_t esp, uint32_t kernel_stack, uint32_t cr3);
void create_user_task(uint32_t entry_point, uint32_t user_stack, uint32_t cr3);

#endif
