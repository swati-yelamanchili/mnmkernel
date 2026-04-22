static inline long my_syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2), "d"(a3) : "memory");
    return ret;
}
static inline long my_syscall2(long n, long a1, long a2) {
    long ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2) : "memory");
    return ret;
}
static inline long my_syscall1(long n, long a1) {
    long ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1) : "memory");
    return ret;
}

void _start() {
    const char file[] = "test.txt";
    // open(file, O_RDONLY) => 5
    long fd = my_syscall2(5, (long)file, 0);
    if (fd < 0) {
        my_syscall1(1, 1);
    }

    char buf[128];
    // read(fd, buf, sizeof(buf)) => 3
    long bytes = my_syscall3(3, fd, (long)buf, sizeof(buf));
    if (bytes > 0) {
        // write(1, buf, bytes) => 4
        my_syscall3(4, 1, (long)buf, bytes);
    }
    
    // close(fd) => 6
    my_syscall1(6, fd);
    
    // exit(0) => 1
    my_syscall1(1, 0);
}
