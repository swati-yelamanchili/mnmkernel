void _start() {
    while (1) {
        const char *str = "I am a new initrd-loaded program!\n";
        __asm__ volatile (
            "mov $4, %%eax\n"
            "mov $1, %%ebx\n"
            "mov %0, %%ecx\n"
            "mov $34, %%edx\n"
            "int $0x80\n"
            :: "r"(str) : "eax", "ebx", "ecx", "edx"
        );
        for (volatile int i = 0; i < 5000000; i++) {}
    }
}
