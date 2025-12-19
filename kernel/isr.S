.macro ISR_NOERRCODE num
  .global isr\num
  isr\num:
    cli
    push $0       # Push a dummy error code to keep the stack frame uniform
    push $\num    # Push the interrupt number
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
  .global isr\num
  isr\num:
    cli
    # Error code is already pushed by CPU
    push $\num    # Push the interrupt number
    jmp isr_common_stub
.endm

.macro IRQ num, vec
  .global irq\num
  irq\num:
    cli
    push $0       # Dummy error code
    push $\vec
    jmp irq_common_stub
.endm

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_ERRCODE 14
ISR_NOERRCODE 128

IRQ 0, 32
IRQ 1, 33

.extern isr_handler
.extern irq_handler

isr_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax # Load kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push %esp      # Pass pointer to registers_t
    call isr_handler
    add $4, %esp   # Cleanup the pointer argument
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    popa
    add $8, %esp  # Cleans up the pushed error code and pushed ISR number
    sti
    iret

irq_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push %esp
    call irq_handler
    mov %eax, %esp   # Switch to the new stack pointer returned by irq_handler!
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    popa
    add $8, %esp
    sti
    iret
