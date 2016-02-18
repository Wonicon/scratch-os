#include "page.h"
#include "debug.h"
#include <inc/mmu.h>

/**
 * 由 ld 定义的符号, 见
 * [start, etext) => 可执行代码, 只读数据
 * [etext, end)   => 初始和未初始化的数据
 */
extern uint8_t start[];  // 程序起始虚拟地址
extern uint8_t etext[];  // 代码段结束虚拟地址(在数据段之前)
extern uint8_t end[];    // 可执行程序结束虚拟地址(开区间)

PDE *kpgdir;

void
init_page(void)
{
    LOG_EXPR(start);
    LOG_EXPR(etext);
    LOG_EXPR(end);
}
