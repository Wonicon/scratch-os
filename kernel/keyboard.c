#include <inc/x86.h>

/**
 * 阻塞型
 * 返回 press 扫描码
 */
uint32_t
get_key(void)
{
    uint32_t scancode;
    uint32_t esccode;

    do {
        scancode = inb(0x60);
        if (scancode == 0xE0) {
            scancode = (scancode << 8) | inb(0x60);
        }
    } while ((scancode & 0x80));

    // wait release
    do {
        esccode = inb(0x60);
        if (esccode == 0xE0) {
            esccode = (esccode << 8) | inb(0x60);
        }
    } while (esccode != (scancode | 0x80));

    return scancode;
}
