#include <stdint.h>

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern void print_serial(const char *str);

void isr_handler(registers_t *regs) {
    (void)regs;
    print_serial("Received Interrupt\n");
}

uint32_t irq_handler(uint32_t esp) {
    registers_t *regs = (registers_t *)esp;
    if (regs->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
    return esp;
}
