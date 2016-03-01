#include <inc/x86.h>
#include <inc/mmu.h>
#include <kernel/segment.h>

/**
 * init_i8259_pic: Initialize the Intel 8259 Programmable Interrupt Controller
 *
 * The default interruption numbers overlap with Intel's reserved 32 interruptions,
 * So the initialization work mainly deals with the interruption number offset and
 * interruption mask bits.
 *
 * The first two bits are for timer and keyboard respectively.
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

    /* We only need the master i8259 chip */

    outb(PORT_PIC_MASTER_CMD, 0x11);   // ICW1
    outb(PORT_PIC_MASTER_DATA, 0x00);  // ICW2
    outb(PORT_PIC_MASTER_DATA, 0x10);  // ICW3
    outb(PORT_PIC_MASTER_DATA, 0x02);  // ICW4_AUTO
}


/**
 * irq_handle: interruption handler
 *
 * All interruption will enter this function
 * and take different actions.
 */
void
irq_handle(StackFrame *frame)
{
    //LOG_EXPR(frame->eip);
    //LOG_EXPR(frame->cs);
    //LOG_EXPR(frame->eflags);
}

/**
 * Interruption Descriptor Table
 */
struct Gatedesc idt[48] = {};

void entry(void);
void
init_idt(void)
{
    int i = 32;
    idt[i].gd_sel = 0x8;
    idt[i].gd_p = 1;
    idt[i].gd_dpl = 0;
    idt[i].gd_s = 0;
    idt[i].gd_rsv1 = 0;
    idt[i].gd_args = 0;
    idt[i].gd_type = STS_IG32;
    idt[i].gd_off_15_0 = (uint32_t)entry & 0x0000ffff;
    idt[i].gd_off_31_16 = ((uint32_t)entry & 0xffff0000) >> 16;
    idt[i + 1] = idt[i];
    idt[0] = idt[i];

    for (int i = 0; i < 48; i++) {
        idt[i] = idt[0];
    }

    DTR idtr = {
        .limit = sizeof(idt) - 1,
        .base  = (uint32_t)idt,
    };

    /**
     * http://wiki.osdev.org/Inline_Assembly/Examples#LIDT
     * 第一个操作数，内存型
     */
    asm volatile ("lidt %0"::"m"(idtr):);
}
