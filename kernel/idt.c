#include "idt.h"

extern void idt_flush(uint32_t);

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags /* | 0x60 */; // uncomment to allow user-mode
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void init_idt(void) {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Zero out the IDT using basic loop (since we don't have memset yet)
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Remap PIC (Interrupt controllers)
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // IRQ0 starts at int 32
    outb(0xA1, 0x28); // IRQ8 starts at int 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Set basic ISRs/IRQs
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E); // Divide by zero
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E); // Debug
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
    
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E); // Timer
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E); // Keyboard

    idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEE); // Syscall (int 0x80) with User Privileges (0xEE)

    idt_flush((uint32_t)&idt_ptr);
}
