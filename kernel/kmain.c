#include <stdint.h>
#include "multiboot.h"

extern void init_gdt(void);
extern void init_idt(void);
extern void init_paging(void);
extern void pmm_init(void);
extern void set_kernel_stack(uint32_t stack);
extern uint32_t load_elf(unsigned char *elf_data);
extern void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack);
extern unsigned char tests_bin_hello32[];

extern void sched_init(void);
extern void init_timer(uint32_t frequency);
extern void create_user_task(uint32_t entry_point, uint32_t user_stack, uint32_t cr3);

extern void init_initrd(uint32_t location);
extern unsigned char* initrd_read_file(const char *filename, uint32_t *length);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void print_serial(const char *str) {
    while (*str) {
        outb(0x3F8, *str++);
    }
}

void print_hex(uint32_t val) {
    char buf[11];
    buf[0] = '0'; 
    buf[1] = 'x'; 
    buf[10] = '\0';
    const char *hex = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        buf[9 - i] = hex[val & 0xF];
        val >>= 4;
    }
    print_serial(buf);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    const char *msg = "MNMKernel Phase 0.5 Booting [Ring 0]...\n";
    uint16_t *vga = (uint16_t*)0xB8000;
    
    // Clear screen basic
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
    
    // Write message to VGA
    for (int i = 0; msg[i] != '\0'; i++) {
        vga[i] = (uint16_t)msg[i] | ((uint16_t)0x0A << 8); // Light green on black
    }
    
    // Write message to Serial (so QEMU with -nographic can show it)
    print_serial(msg);
    print_serial("VGA Memory mapped successfully.\n");
    
    // Initialize PMM
    print_serial("Initializing Physical Memory Manager...\n");
    pmm_init();

    // Initialize Descriptor Tables
    print_serial("Initializing GDT...\n");
    init_gdt();
    print_serial("Initializing IDT...\n");
    init_idt();
    
    print_serial("Initializing Hardware Paging...\n");
    init_paging();

    // Check for Multiboot InitRD
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        struct multiboot_info *mbi = (struct multiboot_info *)addr;
        
        if (mbi->flags & (1 << 3)) {
            struct multiboot_mod_list *mod = (struct multiboot_mod_list *)mbi->mods_addr;
            if (mbi->mods_count > 0) {
                print_serial("Found Initrd at: ");
                print_hex(mod->mod_start);
                print_serial("\n");
                
                // Initialize the generic initrd
                init_initrd(mod->mod_start);
            }
        }
    }

    // Set kernel stack for interrupt trapping properly
    uint32_t kernel_stack;
    __asm__ volatile("mov %%esp, %0" : "=r"(kernel_stack));
    set_kernel_stack(kernel_stack);

    print_serial("Enabling Interrupts...\n");
    __asm__ volatile("sti");

    print_serial("Kernel Ready. Try typing something!\n");

    sched_init();

    print_serial("--- Phase 6: Loading Ring 3 ELFs via InitRD ---\n");
    
    uint32_t len;
    
    // Read hello
    unsigned char *hello_data = initrd_read_file("hello", &len);
    if (!hello_data) {
        print_serial("Could not find hello string!\n");
        while(1);
    }
    // uint32_t elf_entry_hello = load_elf(hello_data);
    uint32_t elf_entry_hello = 0;

    // Stop execution for a second to prevent visual merging!
    
    // Read cat
    /*
    unsigned char *cat_data = initrd_read_file("cat", &len);
    if (!cat_data) {
        print_serial("Could not find cat string!\n");
        while(1);
    }
    uint32_t elf_entry_cat = load_elf(cat_data);
    */
    unsigned char *cat_data = initrd_read_file("cat", &len);
    uint32_t elf_entry_cat = load_elf(cat_data);

    if (elf_entry_cat != 0) {
        print_serial("Spawning tasks and handing over to Task Scheduler...\n");
        
        uint32_t cr3;
        __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
        
        // Task A (hello32)
        // create_user_task(elf_entry_hello, 0x80200000, cr3);

        create_user_task(elf_entry_cat, 0x80300000, cr3);
        
        // Start the timer to kick off scheduling!
        init_timer(5); // 5 Hz to purposefully make it slow so we can read the logs!
    }

    // Halt - the timer IRQ will intercept this natively and switch us out.
    while (1) {
        __asm__ volatile ("hlt");
    }
}
