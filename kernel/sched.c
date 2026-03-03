#include "sched.h"
#include "pmm.h"
#include "paging.h"

// Number of tasks
#define MAX_TASKS 16

tcb_t tasks[MAX_TASKS];
int current_task = -1;
int num_tasks = 0;

extern void print_serial(const char *str);
extern void print_hex(uint32_t val);

void sched_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = 0;
    }
}

uint32_t schedule(uint32_t esp) {
    if (num_tasks == 0) return esp; // Nothing to schedule

    // Save current task state if one is running
    if (current_task != -1) {
        tasks[current_task].esp = esp;
        tasks[current_task].state = 2; // Ready
    }

    // Round robin
    int next_task = (current_task + 1) % num_tasks;
    
    // Skip empty tasks
    while (tasks[next_task].state == 0) {
        next_task = (next_task + 1) % MAX_TASKS;
    }

    current_task = next_task;
    tasks[current_task].state = 1; // Running

    // NOTE: We'll likely need to update TSS esp0 to tasks[current_task].kernel_stack 
    // so interrupts happen on this new task's kernel stack!
    extern void set_kernel_stack(uint32_t stack);
    set_kernel_stack(tasks[current_task].kernel_stack);

    // Switch Page Directory (CR3) if needed!
    // Since currently we are sharing a page directory, we could skip it, or
    // explicitly load it.
    if (tasks[current_task].cr3 != 0) {
        __asm__ volatile("mov %0, %%cr3":: "r"(tasks[current_task].cr3));
    }

    return tasks[current_task].esp;
}

// Function to add a task to scheduler
void add_task(uint32_t esp, uint32_t kernel_stack, uint32_t cr3) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == 0) {
            tasks[i].id = i;
            tasks[i].esp = esp;
            tasks[i].kernel_stack = kernel_stack;
            tasks[i].cr3 = cr3;
            tasks[i].state = 2; // Ready
            
            if (i >= num_tasks) {
                num_tasks = i + 1;
            }
            return;
        }
    }
}

void create_user_task(uint32_t entry_point, uint32_t user_stack, uint32_t cr3) {
    // Allocate a kernel stack for this process
    uint32_t kernel_stack = pmm_alloc_frame() + 4096; // Top of the 4KB frame
    
    // We need to set up the kernel stack so it looks like an interrupt frame
    // that `irq_common_stub` can `popa` and `iret` from!
    uint32_t *stack = (uint32_t *)kernel_stack;
    
    // irq_common_stub expects:
    // SS, ESP, EFLAGS, CS, EIP (Pushed by processor/iret)
    // Error Code, Int_no (Pushed by ISR stub)
    // EAX, ECX, EDX, EBX, ESP(pusha), EBP, ESI, EDI (Pushed by pusha)
    // DS (Pushed by ISR stub)
    
    // Let's build it backwards
    *(--stack) = 0x23; // SS (User Data Segment)
    *(--stack) = user_stack; // ESP
    *(--stack) = 0x202; // EFLAGS (Interrupts enabled)
    *(--stack) = 0x1B; // CS (User Code Segment)
    *(--stack) = entry_point; // EIP
    
    *(--stack) = 0; // Error code (dummy)
    *(--stack) = 0; // int_no (dummy)
    
    // pusha registers
    *(--stack) = 0; // eax
    *(--stack) = 0; // ecx
    *(--stack) = 0; // edx
    *(--stack) = 0; // ebx
    *(--stack) = 0; // esp (ignored by popa)
    *(--stack) = 0; // ebp
    *(--stack) = 0; // esi
    *(--stack) = 0; // edi
    
    *(--stack) = 0x23; // DS (User Data Segment)
    
    // Now add the task
    add_task((uint32_t)stack, kernel_stack, cr3);
}
