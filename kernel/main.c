#include "debug.h"
#include "serial.h"
#include "vesa.h"
#include <inc/x86.h>
#include <inc/types.h>


/**
 * 文字模式字符显式属性格式
 * 参见 https://en.wikipedia.org/wiki/VGA-compatible_text_mode
 * fg 和 bg 是调色版的下标，对应颜色参见 https://en.wikipedia.org/wiki/Video_Graphics_Array#Color_palette
 * blink 对应背景高亮，实际上与 bg 一起组成 16 色的下标
 */
typedef union {
    struct {
        uint8_t fg    : 4;
        uint8_t bg    : 3;
        uint8_t blink : 1;
    };
    uint8_t value;
} Attribute;

void hello_world(void)
{
    /**
     * 代码参考自 http://wiki.0xffffff.org/posts/hurlex-3.html
     * 离线版文档 http://hurlex.0xffffff.org/
     */
    uint8_t *input = (uint8_t *)0xB8000;
    const char *str = "Hello World";

    Attribute attr = {
        .fg    = 0x2,  // green
        .bg    = 0x5,  // magneta
        .blink = 0x0,  // normal
    };

    while (*str) {
        *input++ = *str++;
        *input++ = attr.value;
    }
}

/**
 * 玩耍 virtual 8086 mode 的地方
 */

/**
 * LIDT 使用的数据
 */

#include <inc/mmu.h>

struct __attribute__((packed)) IDTR {
    uint16_t limit; // IDT 的界限, 应该是直接相加的
    uint32_t base;  // IDT 的线性基地址
};

struct __attribute__((packed)) StackFrame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

#define LOG_EXPR(expr) LOG(STR(expr) " is %x", expr)
void vec(struct StackFrame *frame)
{
    /**
     * IVT 与 IDT 是不同的
     * http://wiki.osdev.org/Interrupt_Vector_Table
     */
    LOG_EXPR(frame->eflags);
    LOG_EXPR(frame->eip);
    LOG_EXPR(frame->cs);
    frame->eflags |= 0x23000; // Set VM to 1, IOPL to 3
    struct {
        uint16_t offset;
        uint16_t segment;
    } *ivt = (void *)0;
    frame->cs = ivt[0x3].segment;
    frame->eip = ivt[0x3].offset;
}

struct Gatedesc idt[] = {
    {},
};

void entry(void);

void
vm(void)
{
    // 这个用于 LIDT 的结构体放在栈空间上足够了
    idt[0].gd_sel = 0x8;
    idt[0].gd_p = 1;
    idt[0].gd_dpl = 0;
    idt[0].gd_s = 0;
    idt[0].gd_rsv1 = 0;
    idt[0].gd_args = 0;
    idt[0].gd_type = STS_IG32;
    idt[0].gd_off_15_0 = (uint32_t)entry & 0x0000ffff;
    idt[0].gd_off_31_16 = ((uint32_t)entry & 0xffff0000) >> 16;

    struct IDTR idtr = {
        .limit = sizeof(idt) - 1,
        .base  = (uint32_t)idt,
    };

    LOG("idt.limit = %x, idt.base = %x", idtr.limit, idtr.base);

    /**
     * http://wiki.osdev.org/Inline_Assembly/Examples#LIDT
     * 第一个操作数，内存型
     */
    asm volatile ("lidt %0"::"m"(idtr):);
    asm volatile ("int $0x00");
    LOG("hello!");
}

int
main(void)
{
    init_serial_com1();
    
    LOG("Hello World :-)");

    uint8_t *vmem = (uint8_t *)0xA0000;
    uint8_t pixel = 0x00;
    int i;
    for (i = 0; i < 256; i++) {
        *vmem++ = pixel++;
    }

    vm();
    //get_vbe_info();

    return 0;
}
