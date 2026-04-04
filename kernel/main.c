#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include "syscalls.h"

extern long syscall_dispatch(uint64_t nr, uint64_t arg1, uint64_t arg2, uint64_t arg3, 
                             uint64_t arg4, uint64_t arg5, uint64_t arg6, pid_t caller_pid);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <binary>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        // execve itself will generate a SIGTRAP delivery to the parent
        execve(argv[1], &argv[1], NULL);
        perror("execve");
        exit(1);
    }

    int status;
    // Wait for the SIGTRAP from execve
    waitpid(pid, &status, 0);
    
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);

    while (1) {
        if (ptrace(PTRACE_SYSEMU, pid, 0, 0) == -1) break;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("\n[Kernel] Process exited with %d\n", WEXITSTATUS(status));
            break;
        }
        if (WIFSIGNALED(status)) {
            break;
        }

        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, pid, 0, &regs);

        long syscall_nr = regs.orig_rax;
        
        long ret = syscall_dispatch(syscall_nr, regs.rdi, regs.rsi, regs.rdx, 
                                    regs.r10, regs.r8, regs.r9, pid);

        regs.rax = ret;
        if (syscall_nr == SYS_EXIT || syscall_nr == SYS_EXIT_GROUP) { ptrace(PTRACE_KILL, pid, 0, 0); break; }
        ptrace(PTRACE_SETREGS, pid, 0, &regs);
    }

    return 0;
}
