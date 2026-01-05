#include <stdint.h>
#include "multiboot.h"

extern void init_gdt(void);
extern void init_idt(void);
extern void pmm_init(void);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void print_serial(const char *str) {
    while (*str) outb(0x3F8, *str++);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    (void)magic; (void)addr;
    const char *msg = "MNMKernel Phase 0.5 Booting [Ring 0]...\n";
    uint16_t *vga = (uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    for (int i = 0; msg[i] != '\0'; i++)
        vga[i] = (uint16_t)msg[i] | ((uint16_t)0x0A << 8);
    print_serial(msg);
    print_serial("VGA Memory mapped successfully.\n");

    print_serial("Initializing Physical Memory Manager...\n");
    pmm_init();
    print_serial("Initializing GDT...\n");
    init_gdt();
    print_serial("Initializing IDT...\n");
    init_idt();
    print_serial("Enabling Interrupts...\n");
    __asm__ volatile("sti");
    print_serial("Kernel Ready.\n");

    while (1) __asm__ volatile("hlt");
}
