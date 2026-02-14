#include <stdint.h>

void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack) {
    // 0x23 is the User Data Segment (RPL 3 | Index 4 -> 0x20 | 3)
    // 0x1B is the User Code Segment (RPL 3 | Index 3 -> 0x18 | 3)
    __asm__ volatile (
        "cli\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        "pushl $0x23\n"     // Return SS
        "pushl %0\n"        // Return ESP
        "pushf\n"           // Push EFLAGS
        "popl %%eax\n"
        "or $0x200, %%eax\n"// Enable IF (Interrupts) in EFLAGS
        "pushl %%eax\n"     // Return EFLAGS
        "pushl $0x1B\n"     // Return CS
        "pushl %1\n"        // Return EIP (Entry point)
        
        "iret\n"            // Execute interrupt return, dropping to Ring 3!
        :
        : "r"(user_stack), "r"(entry_point)
        : "eax"
    );
}