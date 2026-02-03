#include "initrd.h"

extern void print_serial(const char *str);
extern void print_hex(uint32_t val);

// The first 4 bytes of our initrd format is the number of files.
// After that follows an array of initrd_file_header_t.
// After the headers, the raw data for the files begins.

static uint32_t initrd_location = 0;
static uint32_t num_headers = 0;
static initrd_file_header_t *file_headers = 0;

void init_initrd(uint32_t location) {
    initrd_location = location;
    num_headers = *((uint32_t *)location);
    file_headers = (initrd_file_header_t *)(location + 4);
    
    print_serial("[INITRD] Initialized at ");
    print_hex(initrd_location);
    print_serial(" with ");
    print_hex(num_headers);
    print_serial(" files.\n");

    for (uint32_t i = 0; i < num_headers; i++) {
        print_serial(" File ");
        print_hex(i);
        print_serial(": ");
        print_serial(file_headers[i].name);
        print_serial("\n");
    }
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

unsigned char* initrd_read_file(const char *filename, uint32_t *length) {
    if (!initrd_location) return 0;

    for (uint32_t i = 0; i < num_headers; i++) {
        if (strcmp(file_headers[i].name, filename) == 0) {
            if (length) *length = file_headers[i].length;
            return (unsigned char *)(initrd_location + file_headers[i].offset);
        }
    }
    
    return 0; // Not found
}
