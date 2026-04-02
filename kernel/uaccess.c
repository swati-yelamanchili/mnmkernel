#define _GNU_SOURCE

#include <sys/uio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>

static ssize_t read_user_memory(pid_t pid, void *dest, const void *src, size_t n) {
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = dest;
    local[0].iov_len = n;
    remote[0].iov_base = (void *)src;
    remote[0].iov_len = n;

    return process_vm_readv(pid, local, 1, remote, 1, 0);
}

static ssize_t write_user_memory(pid_t pid, void *dest, const void *src, size_t n) {
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = (void *)src;
    local[0].iov_len = n;
    remote[0].iov_base = dest;
    remote[0].iov_len = n;

    return process_vm_writev(pid, local, 1, remote, 1, 0);
}

long copy_from_user(pid_t pid, void *dest, const void *src, size_t n) {
    ssize_t nread = read_user_memory(pid, dest, src, n);
    if (nread != (ssize_t)n) return -1;
    return 0;
}

long copy_to_user(pid_t pid, void *dest, const void *src, size_t n) {
    ssize_t nwritten = write_user_memory(pid, dest, src, n);
    if (nwritten != (ssize_t)n) return -1;
    return 0;
}

long copy_string_from_user(pid_t pid, char *dest, const char *src, size_t max_len) {
    if (max_len == 0) {
        return -1;
    }

    ssize_t nread = read_user_memory(pid, dest, src, max_len);
    if (nread <= 0) {
        dest[0] = '\0';
        return -1;
    }

    if (memchr(dest, '\0', (size_t)nread) != NULL) {
        return 0;
    }

    dest[max_len - 1] = '\0';
    return -1; // ENAMETOOLONG/EFAULT equivalent indicator
}
