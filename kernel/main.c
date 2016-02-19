#include "assets/pic.h"
#include "debug.h"
#include "serial.h"
#include "segment.h"
#include "video.h"
#include "page.h"
#include <inc/x86.h>
#include <inc/types.h>
#include <inc/mmu.h>

extern char end[];

/**
 * 阻塞型
 * 返回 press 扫喵码
 */
static inline uint32_t
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

#define swap(x, y) do { typeof(x) z = x; x = y; y = z; } while (0)
int
main(void)
{
    init_serial();
    init_video();
    init_gdt();
    init_idt();
    init_page();
    
    LOG("Hello World :-)");

    LOG("end is %p", end);

    /**
     * Test int
     */
    asm volatile ("int $0x0");

    test_page();
    
    int block_width = 100;
    int block_height = 100;
    int nr_horizonal_blocks = pic_w / block_width;
    int nr_vertical_blocks = pic_h / block_height;
    int nr_blocks = nr_horizonal_blocks * nr_vertical_blocks;

    int blocks[nr_blocks];  // GCC extension

    for (int i = 0; i < nr_blocks; i++) {
        blocks[i] = nr_blocks - 1 - i;
    }

    int scr_h = get_scr_h();
    int scr_w = get_scr_w();

    int block_x = 0;
    int block_y = 0;

    int check_state = 0;  // 0 for init, 1 for exchange
    int check_block_x = 0;
    int check_block_y = 0;

loop:
    init_video_buffer();

    // Draw puzzle
    for (int i = 0; i < nr_blocks; i++) {
        int block_index = blocks[i];
        // Draw a block
        int scr_x = (i % nr_horizonal_blocks) * block_width;
        int scr_y = (i / nr_horizonal_blocks) * block_height;
        int pic_x = (block_index % nr_horizonal_blocks) * block_width;
        int pic_y = (block_index / nr_horizonal_blocks) * block_height;
        for (int off_y = 0; off_y < block_height; off_y++) {
            for (int off_x = 0; off_x < block_width; off_x++) {
                draw_pixel(scr_x + off_x, scr_y + off_y,
                        get_color(pic_bgr, pic_x + off_x, pic_y + off_y, pic_w, pic_h));
            }
        }
    }

    // Draw frame
    for (int i = 0; i < nr_horizonal_blocks + 1; i++) {
        int scr_x = i * block_width;
        draw_column(scr_x, 0, 6, scr_h, 0);
    }
    for (int i = 0; i < nr_vertical_blocks + 1; i++) {
        int scr_y = i * block_height;
        draw_row(0, scr_y, 6, scr_w, 0);
    }


    draw_border(block_x * block_width, block_y * block_height, block_width, block_height, 3, 0x000000ff);

    if (check_state == 1) {
        draw_border(check_block_x * block_width, check_block_y * block_height, block_width, block_height, 3, 0x00ff0000);
    }

    update_screen();

    uint32_t scancode = get_key();
    LOG_EXPR(scancode);

    if (scancode == 0x0039) {
        LOG("hello");
        if (check_state == 0) {
            check_state = 1;
            check_block_x = block_x;
            check_block_y = block_y;
        }
        else {
            check_state = 0;
            int block_first = block_y * nr_horizonal_blocks + block_x;
            int block_second = check_block_y * nr_horizonal_blocks + check_block_x;
            swap(blocks[block_first], blocks[block_second]);
        }
    }

    if      (scancode == 0xe050) block_y++; 
    else if (scancode == 0xe048) block_y--;
    else if (scancode == 0xe04b) block_x--;
    else if (scancode == 0xe04d) block_x++;

    if (block_y >= nr_vertical_blocks) block_y = 0;
    else if (block_y < 0)              block_y = nr_vertical_blocks - 1;

    if (block_x >= nr_horizonal_blocks) block_x = 0;
    else if (block_x < 0)               block_x = nr_horizonal_blocks - 1;

    goto loop;

    return 0;
}
