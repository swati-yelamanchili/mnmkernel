#ifndef INITRD_H
#define INITRD_H

#include <stdint.h>

// A simple flat filesystem structure for the ramdisk
typedef struct {
    char name[64];
    uint32_t offset; // Offset from start of initrd
    uint32_t length;
} initrd_file_header_t;

void init_initrd(uint32_t location);
unsigned char* initrd_read_file(const char *filename, uint32_t *length);

#endif
