// Process tracking service
// Responsible for PID management, exec, and signals.

int proc_service_getpid(int caller_pid) {
    // PID lookup is cheap in the simulator; keeping it accurate avoids a service hop.
    return caller_pid;
}
