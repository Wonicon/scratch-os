#ifndef KERNEL_SEGMENT
#define KERNEL_SEGMENT

#include <inc/types.h>

/**
 * DTR : Descriptor table register
 * 用于 lgdt, lidt 的操作数
 */
typedef struct __attribute__((packed)) {
    uint16_t limit; // 界限, 直接相加，base + limit 为最大值，闭区间
    uint32_t base;  // 线性基地址
} DTR;

#endif  // KERNEL_SEGMENT
