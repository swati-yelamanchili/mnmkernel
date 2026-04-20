static inline long my_syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

static inline long my_syscall1(long n, long a1) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1)
        : "memory"
    );
    return ret;
}

void _start() {
    const char msg[] = "Hello from unmodified static ELF!\n";
    my_syscall3(4, 1, (long)msg, sizeof(msg) - 1); // 4 = sys_write
    my_syscall1(1, 0);                             // 1 = sys_exit
}
