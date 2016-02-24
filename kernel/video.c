#include "video.h"
#include "page.h"
#include "debug.h"
#include <inc/string.h>

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
static uint8_t vmem_buffer[800 * 600 * 3];

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
    LOG_EXPR(mode_info.pitch);
    vmem = (void *)mode_info.physbase;

    // 映射显存区域
    mm_continous_map(get_kpgdir(), (uint32_t)vmem, (uint32_t)vmem, mode_info.Yres * mode_info.pitch);
}

uint32_t
get_scr_w(void)
{
    return mode_info.Xres;
}

uint32_t
get_scr_h(void)
{
    return mode_info.Yres;
}

void
init_video_buffer(void)
{
    memset(vmem_buffer, 0, sizeof(vmem_buffer));
}

void
update_screen(void)
{
    memcpy(vmem, vmem_buffer, sizeof(vmem_buffer));
}

/**
 * 画像素点, 分别指定 B G R
 */
void
draw_pixel_bgr(int x, int y, uint8_t b, uint8_t g, uint8_t r)
{
    if (0 <= x && x < mode_info.Xres && 0 <= y && y < mode_info.Yres) {
        uint8_t *pixel = vmem_buffer + y * mode_info.pitch + x * 3;  // BGR BGR 连续存储
        *pixel++ = b;
        *pixel++ = g;
        *pixel++ = r;
    }
}

/**
 * 画像素点, color 表示原始的颜色值, 格式为 RGB.
 */
void
draw_pixel(int x, int y, int color)
{
    draw_pixel_bgr(x, y, (color >> 16) & 0xff, (color >> 8) & 0xff, (color) & 0xff);
}


/**
 * RGB format in bgr[]:
 *     +-+-+-+-+-+-+
 *  ...|b|g|r|b|g|r|...
 *     +-+-+-+-+-+-+
 *      ^
 *      bgr[y * w * 3 + x * 3]
 * RETURN in format of RGB
 */
uint32_t
get_color(uint8_t bgr[], int x, int y, int w, int h)
{
    int index = (y * w + x) * 3;
    return (bgr[index] << 16) | (bgr[index + 1] << 8) | bgr[index + 2];
}

/**
 * Center-aligned:
 *
 * +-width-+
 * +-+-+-+-+
 * |c|c|c|c|
 * +-+-+-+-+
 *      ^
 *      x
 */
void
draw_column(int x, int y, int width, int length, uint32_t color)
{
    x = x - width / 2;
    for (int off_y = 0; off_y < length; off_y++) {
        for (int off_x = 0; off_x < width; off_x++) {
            draw_pixel(x + off_x, y + off_y, color);
        }
    }
}

void
draw_row(int x, int y, int width, int length, uint32_t color)
{
    y = y - width / 2;
    for (int off_y = 0; off_y < width; off_y++) {
        for (int off_x = 0; off_x < length; off_x++) {
            draw_pixel(x + off_x, y + off_y, color);
        }
    }
}

/**
 * 画边框，厚度在区域内
 *
 * (x,y)
 *   V
 *  +----------------------+--
 *  |  A    thick          |
 *  +--+----------------+--+
 *  |  |                |  |
 *  |B |                |C | h
 *  |  |                |  |
 *  +--+----------------+--+
 *  |  D                   |
 *  +----------------------+--
 *  |         w            |
 */
void
draw_border(int x, int y, int w, int h, int thick, uint32_t color)
{
    // Area A
    draw_row(x, y + thick / 2, thick, w, color);
    // Area D
    draw_row(x, y + h - thick + thick / 2, thick, w, color);
    // Area B
    draw_column(x + thick / 2, y + thick, thick, h - thick * 2, color);
    // Area C
    draw_column(x + w - thick + thick / 2, y + thick, thick, h - thick * 2, color);
}
