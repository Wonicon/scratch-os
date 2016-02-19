#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include <inc/types.h>

#define KEY_UP    0xE048
#define KEY_DOWN  0xE050
#define KEY_LEFT  0xE04B
#define KEY_RIGHT 0xE04D
#define KEY_SPACE 0x39
uint32_t get_key(void);

#endif  // KERNEL_KEYBOARD_H
