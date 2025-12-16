#include "gdt.h"

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;
tss_entry_t tss_entry;

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void init_gdt(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment Ring 0
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment Ring 0
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // Code segment Ring 3
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // Data segment Ring 3

    // Initialize TSS
    for(int i = 0; i < sizeof(tss_entry); i++) {
        ((uint8_t*)&tss_entry)[i] = 0;
    }
    
    // Set up TSS segment (index 5)
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry) - 1;
    gdt_set_gate(5, base, limit, 0x89, 0x40);

    // The kernel stack for Ring 3 interrupts
    tss_entry.ss0 = 0x10; // Kernel Data Segment
    tss_entry.esp0 = 0x0; 

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}
