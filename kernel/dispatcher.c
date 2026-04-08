#include "syscalls.h"
#include "uaccess.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

// Forward declarations
extern long sys_read_fast(int fd, void *buf, uint64_t count, pid_t caller_pid);
extern long sys_write_fast(int fd, const void *buf, uint64_t count, pid_t caller_pid);
extern long sys_close_fast(int fd);
extern long sys_getpid_fast(pid_t caller_pid);
extern long fs_service_openat(int dirfd, const char *pathname, int flags, int mode);
extern long mem_service_mmap(void *addr, uint64_t length, int prot, int flags, int fd, uint64_t offset);

long syscall_dispatch(uint64_t nr, uint64_t arg1, uint64_t arg2, uint64_t arg3, 
                      uint64_t arg4, uint64_t arg5, uint64_t arg6, pid_t caller_pid) {
    (void)arg5; (void)arg6;

    switch (nr) {
        // --- IN-KERNEL (FAST PATH) ---
        case SYS_READ:
            return sys_read_fast((int)arg1, (void*)arg2, arg3, caller_pid);
        case SYS_WRITE:
            return sys_write_fast((int)arg1, (const void*)arg2, arg3, caller_pid);
        case SYS_CLOSE:
            return sys_close_fast((int)arg1);
        case SYS_GETPID:
            return sys_getpid_fast(caller_pid);
        
        case SYS_BRK: 
            return -ENOSYS; // Proper mock needed for real dynamic allocation
        case SYS_MMAP:
            return mem_service_mmap((void *)arg1, arg2, (int)arg3, (int)arg4, (int)arg5, arg6);
            
        case SYS_EXIT:
        case SYS_EXIT_GROUP:
            printf("[Hybrid Kernel] Executing SYS_EXIT. Halting program.\n");
            return 0; // Tracing loop will break

        // --- SERVICE-DELEGATED ---
        case SYS_OPEN:
        case SYS_OPENAT: {
            char path[256];
            int dirfd = (nr == SYS_OPEN) ? AT_FDCWD : (int)arg1;
            const char *upath = (nr == SYS_OPEN) ? (const char*)arg1 : (const char*)arg2;
            int flags = (nr == SYS_OPEN) ? (int)arg2 : (int)arg3;
            int mode  = (nr == SYS_OPEN) ? (int)arg3 : (int)arg4;

            if (copy_string_from_user(caller_pid, path, upath, sizeof(path)) < 0) {
                return -14; // EFAULT
            }
            return fs_service_openat(dirfd, path, flags, mode);
        }

        default:
            printf("[Hybrid Kernel] WARNING: Unimplemented syscall %llu called by PID %d. Returning -ENOSYS.\n", (unsigned long long)nr, caller_pid);
            return -ENOSYS;
    }
}
