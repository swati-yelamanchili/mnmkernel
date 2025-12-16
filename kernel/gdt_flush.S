.global gdt_flush
.global tss_flush

gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %ax      # 0x10 is the offset in the GDT to our data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    # 0x08 is the offset to our code segment. Far jump!
    ljmp $0x08, $flush
flush:
    ret

tss_flush:
    mov $0x28, %ax    # 5th entry in GDT (0x28) is the TSS
    ltr %ax           # Load Task Register
    ret
