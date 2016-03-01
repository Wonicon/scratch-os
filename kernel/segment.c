#include "debug.h"
#include "segment.h"
#include <inc/mmu.h>
#include <inc/string.h>

struct Taskstate tss = {};

struct Segdesc gdt[4] = {};

#define NR_GDT (sizeof(gdt) / sizeof(gdt[0]))

/**
 * 设置描述符的基本属性
 */
void
set_descriptor(struct Segdesc *descriptor, uint32_t type, uint32_t dpl, uint32_t base, uint32_t limit)
{
    memset(descriptor, 0, sizeof(*descriptor));

    // 参数 limit 以字节为单位, 此处修正成以 4KB 为单位
    descriptor->sd_lim_15_0   = (limit >> 12) & 0xffff;
    descriptor->sd_lim_19_16  = limit >> 28;

    descriptor->sd_base_15_0  = base & 0xffff;
    descriptor->sd_base_23_16 = (base >> 16) & 0xff;
    descriptor->sd_base_31_24 = base >> 24;

    descriptor->sd_type       = type;
    descriptor->sd_dpl        = dpl;

    descriptor->sd_s          = 1;  // aplication
    descriptor->sd_p          = 1;  // present
    descriptor->sd_avl        = 0;  // not used
    descriptor->sd_rsv1       = 0;  // intel reserved
    descriptor->sd_g          = 1;  // limit scaled by 4K
    descriptor->sd_db         = 1;  // 32-bit segment
}


/**
 * 初始化描述符表
 */
void
init_gdt(void)
{
    set_descriptor(gdt + 0, 0, 0, 0, 0);                             // NULL
    set_descriptor(gdt + 1, STA_X | STA_R, 0, 0, 0xffffffff);        // KERNEL CODE
    set_descriptor(gdt + 2, STA_W | STA_R, 0, 0, 0xffffffff);        // KERNEL DATA
    set_descriptor(gdt + 3, 9, 0, (uint32_t)&tss, sizeof(tss) - 1);  // TSS

    gdt[3].sd_s = 0;    // Required by TSS

    memset(&tss, 0, sizeof(tss));
    tss.ts_iomb = 104;  // Not check I/O Permission Map

    DTR gdtr = {
        .limit = sizeof(gdt) - 1,
        .base = (uint32_t)gdt,
    };

    asm volatile ("lgdt %0"::"m"(gdtr));
    asm volatile ("ltr %%ax"::"a"(3 << 3));
}
