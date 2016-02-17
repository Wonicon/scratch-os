#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

/**
 * 初始化第一个串口设备
 */
void init_serial(void);

/**
 * 向第一个串口设备输出字符
 */
void serial_putchar(char ch);

#endif  // KERNEL_SERIAL_H
