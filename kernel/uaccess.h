#ifndef UACCESS_H
#define UACCESS_H
#include <sys/types.h>
#include <stddef.h>

long copy_from_user(pid_t pid, void *dest, const void *src, size_t n);
long copy_to_user(pid_t pid, void *dest, const void *src, size_t n);
long copy_string_from_user(pid_t pid, char *dest, const char *src, size_t max_len);

#endif // UACCESS_H
