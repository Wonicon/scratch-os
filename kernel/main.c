#include "assets/pic.h"
#include "debug.h"
#include "serial.h"
#include "segment.h"
#include "video.h"
#include "page.h"
#include <inc/x86.h>
#include <inc/types.h>
#include <inc/mmu.h>

extern char end[];

int
main(void)
{
    init_serial();
    init_video();
    init_gdt();
    init_idt();
    init_page();
    
    LOG("Hello World :-)");

    LOG("end is %p", end);

    for (int y = 0; y < 455; y++) {
        for (int x = 0; x < 600; x++) {
            int index = y * pic_w * 3 + x * 3;
            draw_pixel_bgr(x, y, bgr[index], bgr[index + 1], bgr[index + 2]);
        }
    }

    asm volatile ("int $0x0");

    return 0;
}
