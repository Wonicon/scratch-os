#include "page.h"
#include "debug.h"
#include <inc/mmu.h>
#include <inc/string.h>

/**
 * 由 ld 定义的符号, 见
 * [start, etext) => 可执行代码, 只读数据
 * [etext, end)   => 初始和未初始化的数据
 */
extern uint8_t start[];  // 程序起始虚拟地址
extern uint8_t etext[];  // 代码段结束虚拟地址(在数据段之前)
extern uint8_t end[];    // 可执行程序结束虚拟地址(开区间)

/**
 * Default memory provided by QEMU: 128 MB
 * TODO Query the memory size in runtime
 */
#define PHY_SIZE 0x8000000
#define PG_SIZE 0x1000
#define PG_SIZE_LOG2 12

static PDE *kpgdir = NULL;

PDE *
get_kpgdir(void)
{
    return kpgdir;
}

typedef struct PageInfoBlock PageInfo;
struct PageInfoBlock {
    uint32_t frame;
    uint32_t ref_count;
    PageInfo *next;
};

/**
 * 空闲页链表
 */
PageInfo *free_page_list;

PageInfo page_pool[PHY_SIZE / PG_SIZE];

uint32_t
roundup(uint32_t addr)
{
    return (((addr - 1) >> PG_SIZE_LOG2) + 1) << PG_SIZE_LOG2;
}

uint32_t
rounddown(uint32_t addr)
{
    return addr & (~(PG_SIZE - 1));
}


void
init_page(void)
{
    LOG_EXPR(start);
    LOG_EXPR(etext);
    LOG_EXPR(end);

    uint32_t phy_kernerl = roundup((uint32_t)end);
    uint32_t nr_kernel_page = phy_kernerl / PG_SIZE;
    LOG_EXPR(phy_kernerl);
    LOG_EXPR(nr_kernel_page);

    memset(page_pool, 0, sizeof(page_pool));

    /**
     * 从池中构造链表
     */
    page_pool[nr_kernel_page].frame = nr_kernel_page;
    for (uint32_t i = nr_kernel_page + 1; i < PHY_SIZE / PG_SIZE; i++) {
        page_pool[i].frame = i;
        page_pool[i - 1].next = &page_pool[i];
    }

    free_page_list = &page_pool[nr_kernel_page];

    /**
     * 将所有物理内存映射到自身
     */

    // 申请内核页目录空间
    kpgdir = (PDE *)(alloc_page() << PG_SIZE_LOG2);
    memset(kpgdir, 0, NPDENTRIES * sizeof(PDE));

    // 映射内核和BIOS空间
    mm_continous_map(kpgdir, 0x0, 0x0, (uint32_t)end);

    // 开启分页机制
    uint32_t cr0;
    asm volatile ("mov %0, %%cr3"::"r"(kpgdir));
    asm volatile ("mov %%cr0, %0":"=r"(cr0));
    asm volatile ("mov %0, %%cr0"::"r"(cr0 | 0x80000000));
}

/**
 * 将连续的物理地址映射到连续的虚拟地址 [start, end)
 * 除了页表外不分配页面, 分配的页表不能在映射范围内
 */
void
mm_continous_map(PDE *pgdir, uint32_t paddr_beg, uint32_t vaddr_beg, uint32_t size)
{
    // 页对齐
    paddr_beg = roundup(paddr_beg);
    vaddr_beg = roundup(vaddr_beg);
    uint32_t paddr_end = rounddown(paddr_beg + size);

    // [ kmap_start, kmap_end )
    LinearAddr map_beg = { .val = paddr_beg };
    LinearAddr map_end = { .val = paddr_end };
    LinearAddr vaddr   = { .val = vaddr_beg };

    LOG_EXPR(map_beg.val);
    LOG_EXPR(map_beg.dir_idx);

    LOG_EXPR(map_end.val);
    LOG_EXPR(map_end.dir_idx);

    // NOTE: 虽然 kmap_end 是取不到的地址, 但是页目录下标是可以取到的
    for (uint32_t dir_idx = map_beg.dir_idx; dir_idx <= map_end.dir_idx; dir_idx++) {
        // 分配二级页表
        LinearAddr pgtab_addr = { .frame = alloc_page() };
        PTE *pgtab = (PTE *)pgtab_addr.val;
        memset(pgtab, 0, NPTENTRIES * sizeof(PTE));

        // 填写二级页表项
        LinearAddr pg_addr = { .dir_idx = dir_idx, .tab_idx = 0, .frame = 0 };
        for (uint32_t tab_idx = pg_addr.tab_idx; tab_idx < NPTENTRIES && pg_addr.val < map_end.val; tab_idx++) {
            pgtab[tab_idx].p     = 1;
            pgtab[tab_idx].rw    = 1;
            pgtab[tab_idx].us    = 1;
            pgtab[tab_idx].frame = vaddr.frame;

            // 不能用 .tab_idx++, 会回滚
            vaddr.frame++;
            pg_addr.frame++;
        }

        // 填写一级页表项
        pgdir[dir_idx].p     = 1;
        pgdir[dir_idx].rw    = 1;
        pgdir[dir_idx].us    = 1;
        pgdir[dir_idx].frame = pgtab_addr.frame;
    }
}

/**
 * 返回空闲页的页框号, 同时也是下标
 * 并且 page 的 ref_count 会自增 1
 */
uint32_t
alloc_page(void)
{
    if (free_page_list == NULL) {
        PANIC("No free page!");
    }

    PageInfo *page = free_page_list;
    free_page_list = page->next;
    page->next = NULL;

    TEST(page->ref_count == 0, "A in-use page is allocated from free list!");

    page->ref_count++;

    return page->frame;
}

/**
 * 释放页
 */
void
release_page(uint32_t frame)
{
    TEST(frame < PHY_SIZE / PG_SIZE, "page frame exceeds limit!");

    PageInfo *page = &page_pool[frame];
    page->ref_count--;

    if (page->ref_count == 0) {
        page->next = free_page_list;
        free_page_list = page->next;
    }
}

void
test_page(void)
{
    uint32_t frame_buf[0xff];
    int n = 0xff;
    for (int i = 0; i < n; i++) {
        frame_buf[i] = alloc_page();
    }
    for (int i = 0; i < n; i++) {
        release_page(frame_buf[i]);
    }
    PageInfo *page = free_page_list;
    for (int i = 0; i < 0xf; i++) {
        LOG_EXPR(page->frame);
        page = page->next;
    }
}

/**
 * 返回的是物理地址
 */
uint32_t
mm_malloc(uint32_t vaddr, uint32_t size);
