#ifndef SYSCALLS_H
#define SYSCALLS_H

/* Validated Linux x86_64 Syscall Numbers */
#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_OPEN      2
#define SYS_CLOSE     3
#define SYS_FSTAT     5
#define SYS_LSEEK     8
#define SYS_MMAP      9
#define SYS_MUNMAP    11
#define SYS_BRK       12
#define SYS_GETPID    39
#define SYS_CLONE     56
#define SYS_EXECVE    59
#define SYS_EXIT      60
#define SYS_WAIT4     61
#define SYS_EXIT_GROUP 231
#define SYS_OPENAT    257

/* System Error Codes */
#define EBADF         9
#define ENOMEM        12
#define EFAULT        14
#define EINVAL        22
#define ENOSYS        38

#endif // SYSCALLS_H
