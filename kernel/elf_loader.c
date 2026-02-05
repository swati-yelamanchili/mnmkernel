#include <stdint.h>

extern void print_serial(const char *str);
extern void print_hex(uint32_t val);

struct elf32_header {
    uint8_t  magic[4];
    uint8_t  bitness;
    uint8_t  endianness;
    uint8_t  version;
    uint8_t  abi;
    uint8_t  abi_version;
    uint8_t  pad[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version_2;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed));

struct elf32_phdr {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} __attribute__((packed));

uint32_t load_elf(unsigned char *elf_data) {
    struct elf32_header *hdr = (struct elf32_header *)elf_data;

    // Verify magic (\x7fELF)
    if (hdr->magic[0] != 0x7F || hdr->magic[1] != 'E' || 
        hdr->magic[2] != 'L' || hdr->magic[3] != 'F') {
        print_serial("ELF MAGIC NOT FOUND!\n");
        return 0;
    }

    print_serial("[ELF Loader] Found valid ELF file. Entry: ");
    print_hex(hdr->entry);
    print_serial("\n");

    struct elf32_phdr *phdrs = (struct elf32_phdr *)(elf_data + hdr->phoff);

    for (int i = 0; i < hdr->phnum; i++) {
        if (phdrs[i].type == 1) { // PT_LOAD
            print_serial("[ELF Loader] PT_LOAD segment found. VAddr: ");
            print_hex(phdrs[i].vaddr);
            print_serial("\n");

            uint8_t *dest = (uint8_t *)phdrs[i].vaddr;
            uint8_t *src = elf_data + phdrs[i].offset;

            // Simple block copy. 
            // Note: Since this targets 0x80000000, IF the page is missing, 
            // the copy will immediately trigger the Page Fault handler, 
            // which will seamlessly allocate hardware RAM and return here!
            for (uint32_t j = 0; j < phdrs[i].filesz; j++) {
                dest[j] = src[j];
            }

            // Zero out bss padding.
            for (uint32_t j = phdrs[i].filesz; j < phdrs[i].memsz; j++) {
                dest[j] = 0;
            }
        }
    }

    return hdr->entry; // Return extracted physical entry point target
}