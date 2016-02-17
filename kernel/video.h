#ifndef KERNEL_VIDEO_H
#define KERNEL_VIDEO_H

#include <inc/types.h>

void init_video(void);

void draw_pixel_bgr(int x, int y, uint8_t b, uint8_t g, uint8_t r);

void draw_pixel(int x, int y, int color);

#endif  // KERNEL_VIDEO_H
