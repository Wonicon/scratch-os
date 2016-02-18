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

PDE *kpgdir = NULL;

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
