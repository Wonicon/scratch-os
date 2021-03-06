#ifndef KERNEL_PAGE_H
#define KERNEL_PAGE_H

#include <inc/types.h>

/**
 * PTE => Page Table Entry
 * frame -> page frame corresponding to virtual address
 */
typedef union __attribute__((__packed__)) {
    struct {
        uint16_t p     : 1;  // Present
        uint16_t rw    : 1;  // Read / Write
        uint16_t us    : 1;  // User / Superuser
        uint16_t       : 2;  // Intel reserved
        uint16_t a     : 1;  // Also available ?
        uint16_t d     : 1;  // Dirty
        uint16_t       : 2;  // Intel reserved
        uint16_t avail : 3;  // Available for systems programmer use
        uint32_t frame : 20;
    };
    uint32_t val;
} PTE;

/**
 * PDE => Page Directory Entry
 * frame -> page frame storing page table
 */
typedef PTE PDE;

/**
 * 分解线性地址字段
 */
typedef union {
    struct {
        uint32_t offset  : 12;
        uint32_t tab_idx : 10;
        uint32_t dir_idx : 10;
    };
    struct {
        uint32_t         : 12;
        uint32_t frame   : 20;
    };
    uint32_t val;
} LinearAddr;

PDE *get_kpgdir(void);

void init_page(void);

void mm_continous_map(PDE *pgdir, uint32_t paddr_beg, uint32_t vaddr_beg, uint32_t size);

uint32_t alloc_page(void);

void release_page(uint32_t frame);

void test_page(void);

#endif  // KERNEL_PAGE_H
