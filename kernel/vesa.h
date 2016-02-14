#ifndef KERNEL_VESA_H
#define KERNEL_VESA_H

#include <inc/types.h>

struct VbeInfoBlock {
    char VbeSignature[4];             // == "VESA"
    uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];         // isa vbeFarPtr
    uint8_t Capabilities[4];
    uint16_t VideoModePtr[2];         // isa vbeFarPtr
    uint16_t TotalMemory;             // as # of 64KB blocks
} __attribute__((packed));

typedef struct VbeInfoBlock VbeInfo;

void get_vbe_info(void);

#endif  // KERNEL_VESA_H
