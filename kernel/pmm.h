#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// Initializes the Physical Memory Manager bitmap
void pmm_init(void);

// Allocates a 4KB physical frame and returns its physical address
uint32_t pmm_alloc_frame(void);

// Frees a previously allocated physical frame
void pmm_free_frame(uint32_t phys_addr);

#endif // PMM_H
