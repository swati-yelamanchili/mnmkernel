#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// This module represents the isolated File System Service.
// In the final architecture, it runs as a separate process and communicates via IPC.

long fs_service_openat(int dirfd, const char *pathname, int flags, int mode) {
    // For prototype simulation, backed by host POSIX
    long ret = openat(dirfd, pathname, flags, mode);
    if (ret < 0) return -errno;
    return ret;
}

int fs_service_close(int fd) {
    return close(fd) < 0 ? -errno : 0;
}

long fs_service_read(int fd, void *buf, uint64_t count) {
    long ret = read(fd, buf, count);
    if (ret < 0) return -errno;
    return ret;
}

long fs_service_write(int fd, const void *buf, uint64_t count) {
    long ret = write(fd, buf, count);
    if (ret < 0) return -errno;
    return ret;
}
