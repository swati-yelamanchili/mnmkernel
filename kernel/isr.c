#include <stdint.h>
#include "paging.h"
#include "sched.h"

extern unsigned char* initrd_read_file(const char *filename, uint32_t *length);

// Simple global VFS state for baremetal testing
static unsigned char* open_file_data = 0;
static uint32_t open_file_len = 0;
static uint32_t open_file_pos = 0;

extern void print_serial(const char *str); // From kmain.c
extern void print_hex(uint32_t val);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void isr_handler(registers_t *regs) {
    if (regs->int_no == 14) {
        page_fault_handler((page_fault_registers_t *)regs);
        return;
    }
    
    if (regs->int_no == 128) {
        if (regs->eax == 4) { // sys_write
            uint32_t fd = regs->ebx;
            const char* buf = (const char*)regs->ecx;
            uint32_t len = regs->edx;
            if (fd == 1) { // stdout
                for(uint32_t i=0; i<len; i++){
                    __asm__ volatile ("outb %0, %1" : : "a"(buf[i]), "Nd"((uint16_t)0x3F8));
                }
                regs->eax = len;
            } else {
                regs->eax = -1;
            }
        } else if (regs->eax == 1) { // sys_exit
            print_serial("\n[Syscall] Process Executed sys_exit. Halting.\n");
            __asm__ volatile ("cli; hlt");
        } else if (regs->eax == 5) { // sys_open
            const char* filename = (const char*)regs->ebx;
            uint32_t len = 0;
            print_serial("\n[Syscall] Process Executed sys_open. Opening: ");
            print_serial(filename);
            print_serial("\n");
            unsigned char* file = initrd_read_file(filename, &len);
            if (file) {
                open_file_data = file;
                open_file_len = len;
                open_file_pos = 0;
                regs->eax = 3;
            } else {
                regs->eax = -1;
            }
        } else if (regs->eax == 3) { // sys_read
            uint32_t fd = regs->ebx;
            char* buf = (char*)regs->ecx;
            uint32_t count = regs->edx;
            if (fd == 3 && open_file_data != 0){
                uint32_t bytes_to_read = count;
                if (open_file_pos + bytes_to_read > open_file_len) {
                    bytes_to_read = open_file_len - open_file_pos;
                }
                for(uint32_t i=0; i<bytes_to_read; i++){
                    buf[i] = open_file_data[open_file_pos++];
                }
                regs->eax = bytes_to_read;
            } else {
                regs->eax = -1;
            }
        } else if (regs->eax == 6) { // sys_close
            uint32_t fd = regs->ebx;
            if (fd == 3) {
                open_file_data = 0;
                open_file_len = 0;
                open_file_pos = 0;
                regs->eax = 0;
            } else {
                regs->eax = -1;
            }
        } else {
            regs->eax = -38;
        }

        return;
    }

    print_serial("Received Interrupt\n");
}

uint32_t irq_handler(uint32_t esp) {
    registers_t *regs = (registers_t *)esp;
    if (regs->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
    if (regs->int_no == 32) esp = schedule(esp);
    return esp;
}
