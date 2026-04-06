#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "syscalls.h"
#include "uaccess.h"

#define FASTPATH_STACK_BUF 4096

static void *alloc_bounce_buffer(size_t count, char stack_buf[FASTPATH_STACK_BUF]) {
    if (count <= FASTPATH_STACK_BUF) {
        return stack_buf;
    }

    return malloc(count);
}

static void free_bounce_buffer(void *buf, char stack_buf[FASTPATH_STACK_BUF]) {
    if (buf != stack_buf) {
        free(buf);
    }
}

long sys_read_fast(int fd, void *buf, uint64_t count, pid_t caller_pid) {
    char stack_buf[FASTPATH_STACK_BUF];
    size_t local_count;
    void *local_buf;
    long ret;

    if (fd < 0) return -EBADF;
    if (count == 0) return 0;
    if (count > SIZE_MAX) return -EINVAL;

    local_count = (size_t)count;
    local_buf = alloc_bounce_buffer(local_count, stack_buf);
    if (!local_buf) return -ENOMEM;

    ret = read(fd, local_buf, local_count);
    if (ret < 0) {
        int saved_errno = errno;
        free_bounce_buffer(local_buf, stack_buf);
        return -saved_errno;
    }

    if (ret > 0 && copy_to_user(caller_pid, buf, local_buf, (size_t)ret) < 0) {
        free_bounce_buffer(local_buf, stack_buf);
        return -EFAULT;
    }

    free_bounce_buffer(local_buf, stack_buf);
    return ret;
}

long sys_write_fast(int fd, const void *buf, uint64_t count, pid_t caller_pid) {
    char stack_buf[FASTPATH_STACK_BUF];
    size_t local_count;
    void *local_buf;
    long ret;

    if (fd < 0) return -EBADF;
    if (count == 0) return 0;
    if (count > SIZE_MAX) return -EINVAL;

    local_count = (size_t)count;
    local_buf = alloc_bounce_buffer(local_count, stack_buf);
    if (!local_buf) return -ENOMEM;

    if (copy_from_user(caller_pid, local_buf, buf, local_count) < 0) {
        free_bounce_buffer(local_buf, stack_buf);
        return -EFAULT;
    }

    ret = write(fd, local_buf, local_count);
    if (ret < 0) {
        int saved_errno = errno;
        free_bounce_buffer(local_buf, stack_buf);
        return -saved_errno;
    }

    free_bounce_buffer(local_buf, stack_buf);
    return ret;
}

long sys_close_fast(int fd) {
    if (fd < 0) return -EBADF;
    if (close(fd) < 0) return -errno;
    return 0;
}

long sys_getpid_fast(pid_t caller_pid) {
    return caller_pid;
}
