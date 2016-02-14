#include "vesa.h"
#include "debug.h"

void
get_vbe_info(void)
{
    /**
     * TODO
     * INT 10H 不能在保护模式下执行
     */
    VbeInfo vbe = {};
    asm volatile ("int $0x10"::"a"(0x004f), "D"(&vbe));
}
