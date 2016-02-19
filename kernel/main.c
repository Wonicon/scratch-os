#include "debug.h"
#include "serial.h"
#include "segment.h"
#include "video.h"
#include "page.h"
#include <inc/x86.h>
#include <inc/types.h>
#include <inc/mmu.h>

void game_mainloop(void);

int
main(void)
{
    init_serial();
    init_video();
    init_gdt();
    init_idt();
    init_page();
    
    LOG("Hello World :-)");

    /**
     * Test int
     */
    asm volatile ("int $0x0");

    test_page();

    game_mainloop();

    return 0;
}
