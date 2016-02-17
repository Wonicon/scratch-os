#include <inc/x86.h>

/**
 * 第一个串口设备 COM1 的端口号
 * 对应 qemu 第一个 -serial 选项指定的设备
 *
 * 核心代码来自 http://wiki.osdev.org/Serial_Ports
 */
#define COM1 0x3f8

static void
init_serial_com(int port)
{
    outb(port + 1, 0x00);  // Disable all interrupts
    outb(port + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outb(port + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outb(port + 1, 0x00);  //                  (hi byte)
    outb(port + 3, 0x03);  // 8 bits, no parity, one stop bit
    outb(port + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outb(port + 4, 0x0B);  // IRQs enabled, RTS/DSR set
}

static int
is_transmit_empty(int port)
{
    return inb(port + 5) & 0x20;
}

static void
write_serial(int port, char ch)
{
    while (is_transmit_empty(port) == 0);
    outb(port, ch);
}

void
init_serial(void)
{
    init_serial_com(COM1);
}

void
serial_putchar(char ch)
{
    write_serial(COM1, ch);
}
