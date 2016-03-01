#include "debug.h"
#include "serial.h"
#include "video.h"
#include "page.h"
#include "segment.h"
#include <inc/x86.h>
#include <inc/types.h>
#include <inc/mmu.h>

void init_gdt(void);
void init_idt(void);
void init_i8259_pic(void);
void game_mainloop(void);

int
main(void)
{
    init_serial();
    LOG("Hello World :-)");

    init_gdt();
    init_idt();
    init_page();
    init_video();

    init_i8259_pic();
    asm volatile ("sti");

    // Test int
    asm volatile ("int $0x0");

    test_page();

    LOG("Game start");

    StackFrame thread_task = {
        .eip    = (uint32_t)game_mainloop,
        .cs     = 0x8,
        .eflags = 0x2,
    };

    asm volatile (
        "mov %0, %%eax;"
        "mov %%eax, %%esp;"
        "iret"
        :
        : "r"(&thread_task)
        : "eax"
    );

    return 0;
}
