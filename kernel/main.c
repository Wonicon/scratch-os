#include "debug.h"
#include "serial.h"
#include "segment.h"
#include "video.h"
#include "page.h"
#include <inc/x86.h>
#include <inc/types.h>
#include <inc/mmu.h>

void game_mainloop(void);

/**
 * init_i8259_pic: Initialize the Intel 8259 Programmable Interrupt Controller
 */
#define PORT_PIC_MASTER_CMD 0x20
#define PORT_PIC_SLAVE_CMD  0xA0
#define PORT_PIC_MASTER_DATA (PORT_PIC_MASTER_CMD + 1)
#define PORT_PIC_SLAVE_DATA (PORT_PIC_SLAVE_CMD + 1)

void
init_i8259_pic(void)
{
    outb(PORT_PIC_MASTER_DATA, 0xFE);
    outb(PORT_PIC_SLAVE_DATA, 0xFF);
    outb(PORT_PIC_MASTER_CMD, 0x11);
    outb(PORT_PIC_MASTER_DATA, 0x00);
    outb(PORT_PIC_MASTER_DATA, 0x10);
    outb(PORT_PIC_MASTER_DATA, 0x02);  // ICW4_AUTO
}

int
main(void)
{
    init_serial();
    LOG("Hello World :-)");

    init_gdt();
    init_idt();
    init_page();
    init_video();
    

    // Test int
    asm volatile ("int $0x0");

    test_page();

    LOG("Game start");
    init_i8259_pic();
    asm volatile ("sti");
    game_mainloop();

    return 0;
}
