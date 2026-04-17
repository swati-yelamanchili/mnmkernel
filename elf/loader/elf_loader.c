#include <stdint.h>
#include <stddef.h>

// Static ELF definitions
#define ELFMAG "\177ELF"

typedef struct {
    unsigned char e_ident[16];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint64_t      e_entry;       // Entry point
    uint64_t      e_phoff;
    uint64_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf64_Ehdr;

// Minimal loader scaffold
int load_elf(const char* filepath, uint64_t* entry_point) {
    // Read header, validate magic, map segments, setup stack...
    // Returns 0 on success.
    return 0;
}
