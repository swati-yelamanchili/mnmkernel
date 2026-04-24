static inline long my_syscall0(long n) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long my_syscall1(long n, long a1) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long my_syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long my_syscall4(long n, long a1, long a2, long a3, long a4) {
    long ret;
    register long r10 __asm__("r10") = a4;

    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static int append_string(char *dest, int pos, const char *src) {
    while (*src) {
        dest[pos++] = *src++;
    }
    return pos;
}

static int append_u64(char *dest, int pos, unsigned long value) {
    char scratch[32];
    int len = 0;

    if (value == 0) {
        dest[pos++] = '0';
        return pos;
    }

    while (value > 0) {
        scratch[len++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (len > 0) {
        dest[pos++] = scratch[--len];
    }

    return pos;
}

void _start(void) {
    char out[256];
    char buf[96];
    const char path[] = "README.md";
    long pid = my_syscall0(39);
    int len = 0;

    if (pid <= 1) {
        my_syscall1(60, 2);
    }

    len = append_string(out, len, "pid=");
    len = append_u64(out, len, (unsigned long)pid);
    out[len++] = '\n';
    my_syscall3(1, 1, (long)out, len);

    long fd = my_syscall4(257, -100, (long)path, 0, 0);
    if (fd < 0) {
        my_syscall1(60, 3);
    }

    long bytes = my_syscall3(0, fd, (long)buf, sizeof(buf));
    if (bytes > 0) {
        my_syscall3(1, 1, (long)buf, bytes);
    }

    my_syscall1(3, fd);
    my_syscall1(60, 0);
}
