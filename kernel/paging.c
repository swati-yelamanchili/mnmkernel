#include "paging.h"
#include "pmm.h"

extern void print_serial(const char *str);
extern void print_hex(uint32_t val);

// A page directory and page table must strictly be 4KB aligned on x86
uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

// Create a pool of blank page tables to attach dynamically to any directory index.
uint32_t dynamic_page_tables[16][1024] __attribute__((aligned(4096)));
uint32_t num_dynamic_tables = 0;

void init_paging(void) {
    print_serial("Setting up Page Directory...\n");

    // 1. Initialize page directory - mark everything as not-present, read/write, supervisor (0x02)
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    // 2. Identity map the first 4MB of physical memory.
    // Our kernel is loaded at 1MB, and VGA memory is at 0xB8000, which are both within this lowest 4MB!
    // 1024 pages * 4096 bytes per page = 4,194,304 bytes (4MB)
    for (int i = 0; i < 1024; i++) {
        // Physical Address | Present=1 | Read/Write=1 | Supervisor=0 -> (bit 0 and bit 1 = 1) -> 0x03
        first_page_table[i] = (i * 0x1000) | 3; 
    }

    // 3. Put the first page table into the very first page directory entry.
    // This connects memory locations 0x00000000 -> 0x003FFFFF 
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    // We no longer pre-stage directory 512! We let page_fault_handler dynamically assign them.

    // 4. Load the physical address of the page directory into the CR3 register
    print_serial("Loading CR3...\n");
    __asm__ volatile("mov %0, %%cr3":: "r"(page_directory));

    // 5. Read the CR0 control register, flip the Paging Enable bit (bit 31), and write it back!
    print_serial("Flipping CR0 Paging Bit...\n");
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Bit 31
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));

    print_serial("Hardware Paging Enabled!\n");
}

void page_fault_handler(page_fault_registers_t *regs) {
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // Calculate indexes
    uint32_t dir_idx = faulting_address >> 22;
    uint32_t table_idx = (faulting_address >> 12) & 0x03FF;

    print_serial("[MMU] Page Fault Trapped! Reaching for virtual address ");
    print_hex(faulting_address);
    print_serial("\n");

    // Allow dynamic allocation for any address above 4MB (dir_idx >= 1)
    if (dir_idx >= 1) {
        // Check if page directory entry exists
        if (!(page_directory[dir_idx] & 0x1)) {
            // Need a new page table!
            if (num_dynamic_tables >= 16) {
                print_serial("VMM PANIC: Out of kernel page tables!\n");
                while(1);
            }
            uint32_t *new_table = (uint32_t *)dynamic_page_tables[num_dynamic_tables++];
            for (int i = 0; i < 1024; i++) {
                new_table[i] = 0x06; // user, r/w, not present
            }
            page_directory[dir_idx] = ((uint32_t)new_table) | 7; // Present, R/W, User
            
            // Re-load CR3 to flush directory cache just in case
            uint32_t cr3_val;
            __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_val));
            __asm__ volatile("mov %0, %%cr3" :: "r"(cr3_val));
        }

        uint32_t *target_table = (uint32_t *)(page_directory[dir_idx] & 0xFFFFF000);

        // Attempt to allocate physical frame
        uint32_t phys_addr = pmm_alloc_frame();
        if (phys_addr == 0) {
            print_serial("VMM PANIC: Out of Physical Memory!\n");
            while(1); // Halt
        }

        print_serial("[MMU] Granted physical frame ");
        print_hex(phys_addr);
        print_serial("\n");

        // Map the frame in the page table (Present=1, R/W=1, User=1 -> 7)
        target_table[table_idx] = phys_addr | 7;

        // Invalidate the TLB cache covering this virtual address
        __asm__ volatile("invlpg (%0)" ::"r" (faulting_address) : "memory");

        // Safely Zero-out the newly allocated memory frame through its mapped VA
        uint32_t aligned_virt = faulting_address & 0xFFFFF000;
        char *ptr = (char *)aligned_virt;
        for (int i = 0; i < 4096; i++) {
            ptr[i] = 0;
        }

        print_serial("[MMU] Transparently Returning execution to application.\n");
    } else {
        print_serial("VMM PANIC: Segmentation Fault on Unhandled Boundary.\n");
        // Decode standard error states for debug
        int present = !(regs->err_code & 0x1); // Not present
        int rw = regs->err_code & 0x2;         // Write attempted
        int us = regs->err_code & 0x4;         // User mode

        print_serial("Attributes: ");
        if (present) print_serial("[Not-Present] ");
        if (rw) print_serial("[Write-Violation] ");
        if (us) print_serial("[Ring-3 Violation] ");
        print_serial("\n");
        
        while(1) {
            __asm__ volatile("hlt");
        }
    }
}
