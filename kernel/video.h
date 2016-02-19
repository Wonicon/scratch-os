#ifndef KERNEL_VIDEO_H
#define KERNEL_VIDEO_H

#include <inc/types.h>

void init_video(void);

uint32_t get_scr_w(void);

uint32_t get_scr_h(void);

void draw_pixel_bgr(int x, int y, uint8_t b, uint8_t g, uint8_t r);

void draw_pixel(int x, int y, int color);

uint32_t get_color(uint8_t bgr[], int x, int y, int w, int h);

void draw_column(int x, int y, int width, int length, uint32_t color);

void draw_row(int x, int y, int width, int length, uint32_t color);

void init_video_buffer(void);

void update_screen(void);

#endif  // KERNEL_VIDEO_H
