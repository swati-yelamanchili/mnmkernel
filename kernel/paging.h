#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Matches the registers struct in isr.c
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} page_fault_registers_t;

// Initializes the page directory and tables, then enables paging in CR0
void init_paging(void);

// Handles page faults (ISR 14), automatically allocating memory for the heap/userspace
void page_fault_handler(page_fault_registers_t *regs);

#endif // PAGING_H
