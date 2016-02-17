#include "video.h"

/**
 * 显式模式具体信息
 * 来自 wiki.osdev.org
 * 实际需要关心的字段是 pitch, Xres, Yres, physbase
 */
typedef struct {
    uint16_t attributes;
    uint8_t winA,winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint16_t offset;
    uint16_t base;
    uint16_t pitch;       // 每行字节总数
    uint16_t Xres, Yres;  // 分辨率
    uint8_t Wchar, Ychar, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;
    uint32_t physbase;    // 显存物理地址 (值看起来想虚拟地址, 但是直接当做物理地址使用)
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed)) ModeInfoBlock;


static ModeInfoBlock mode_info = {};

/**
 * 显存地址
 */
static uint8_t *vmem = NULL;

/**
 * 将 mode info 保存到内核空间
 * 并设置显存地址
 */
void
init_video(void)
{
    /**
     * 0xf000 是 boot loader 暂存 ModeInfoBlock 的地址
     * 此片区域可以保证没有预设内容, 但要小心代码对这里进行的写入
     */
    mode_info = *(ModeInfoBlock *)0xf000;
    vmem = (void *)mode_info.physbase;
}

/**
 * 画像素点, 分别指定 B G R
 */
void
draw_pixel_bgr(int x, int y, uint8_t b, uint8_t g, uint8_t r)
{
    if (0 <= x && x < mode_info.Xres && 0 <= y && y < mode_info.Yres) {
        uint8_t *pixel = vmem + y * mode_info.pitch + x * 3;  // BGR BGR 连续存储
        *pixel++ = b;
        *pixel++ = g;
        *pixel++ = r;
    }
}

/**
 * 画像素点, color 表示原始的颜色值, 格式为 BGR.
 */
void
draw_pixel(int x, int y, int color)
{
    draw_pixel_bgr(x, y, color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff);
}

