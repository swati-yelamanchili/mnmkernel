#include "pmm.h"

// 1024 blocks of 32 bits = 32768 frames = 128MB of RAM map
#define PMM_BLOCKS 1024 

static uint32_t pmm_bitmap[PMM_BLOCKS];

void pmm_init(void) {
    for (int i = 0; i < PMM_BLOCKS; i++) {
        pmm_bitmap[i] = 0;
    }
    // Mark the first 4MB (1024 frames) as USED.
    // This physically protects the kernel code, VGA memory, and identity mapped structures.
    // 1024 frames = 32 blocks of 32 bits.
    for (int i = 0; i < 32; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < PMM_BLOCKS; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(pmm_bitmap[i] & (1 << j))) {
                    // Found a free frame, mark it as used
                    pmm_bitmap[i] |= (1 << j);
                    return (i * 32 + j) * 4096;
                }
            }
        }
    }
    return 0; // Out of memory (returns Physical Address 0x0, which is dangerous but standard for failure)
}

void pmm_free_frame(uint32_t phys_addr) {
    uint32_t frame = phys_addr / 4096;
    uint32_t i = frame / 32;
    uint32_t j = frame % 32;
    pmm_bitmap[i] &= ~(1 << j);
}
