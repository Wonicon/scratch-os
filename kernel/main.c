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
    draw_border(0, 0, block_width, block_height, 3, 0x000000ff);

    update_screen();
    goto loop;

    return 0;
}
