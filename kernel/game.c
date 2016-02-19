#include "video.h"
#include "keyboard.h"
#include "assets/pic.h"
#include "debug.h"

typedef enum {
    GAME_SEL_NONE,
    GAME_SEL_SELECTED,
} SelState;

typedef struct {
    uint8_t *image;
    int image_w;
    int image_h;
    int *block;
    int block_x;
    int block_y;
    int select_block_x;
    int select_block_y;
    int nr_block_h;
    int nr_block_v;
    int block_w;
    int block_h;
    SelState select;
} GameState;

void
update_frame(const GameState *game)
{
    init_video_buffer();

    // Draw puzzle
    int nr_block = game->nr_block_h * game->nr_block_v;
    for (int i = 0; i < nr_block; i++) {
        int block_index = game->block[i];
        // Draw a block
        int scr_x = (i % game->nr_block_h) * game->block_w;
        int scr_y = (i / game->nr_block_h) * game->block_h;
        int pic_x = (block_index % game->nr_block_h) * game->block_w;
        int pic_y = (block_index / game->nr_block_h) * game->block_h;
        for (int off_y = 0; off_y < game->block_h; off_y++) {
            for (int off_x = 0; off_x < game->block_w; off_x++) {
                draw_pixel(scr_x + off_x, scr_y + off_y,
                        get_color(game->image, pic_x + off_x, pic_y + off_y, game->image_w, game->image_h));
            }
        }
    }

    // Draw frame
    for (int i = 0; i < game->nr_block_h + 1; i++) {
        int scr_x = i * game->block_w;
        draw_column(scr_x, 0, 6, get_scr_h(), 0);
    }
    for (int i = 0; i < game->nr_block_v + 1; i++) {
        int scr_y = i * game->block_h;
        draw_row(0, scr_y, 6, get_scr_w(), 0);
    }

    draw_border(game->block_x * game->block_w, game->block_y * game->block_h,
                game->block_w, game->block_h, 3, 0x000000ff);

    if (game->select == 1) {
        draw_border(game->select_block_x * game->block_w, game->select_block_y * game->block_h,
                    game->block_w, game->block_h, 3, 0x00ff0000);
    }

    update_screen();
}

void
update_game_state(GameState *game)
{
    uint32_t scancode = get_key();

    if (scancode == KEY_SPACE) {
        if (game->select == 0) {
            game->select = 1;
            game->select_block_x = game->block_x;
            game->select_block_y = game->block_y;
        }
        else {
            game->select = 0;
            int block_first = game->block_y * game->nr_block_h + game->block_x;
            int block_second = game->select_block_y * game->nr_block_h + game->select_block_x;
            swap(game->block[block_first], game->block[block_second]);
        }
    }

    if      (scancode == KEY_DOWN)  game->block_y++; 
    else if (scancode == KEY_UP)    game->block_y--;
    else if (scancode == KEY_LEFT)  game->block_x--;
    else if (scancode == KEY_RIGHT) game->block_x++;

    if (game->block_y >= game->nr_block_v) game->block_y = 0;
    else if (game->block_y < 0)            game->block_y = game->nr_block_v - 1;

    if (game->block_x >= game->nr_block_h) game->block_x = 0;
    else if (game->block_x < 0)      game->block_x = game->nr_block_h - 1;

}

int
check_valid(const GameState *game)
{
    int *block = game->block;
    int nr_block = game->nr_block_h * game->nr_block_v;

    for (int i = 0; i < nr_block - 1; i++) {
        if (block[i] > block[i + 1]) {
            return 0;
        }
    }

    return 1;
}

void
game_mainloop(void)
{
    GameState game = {
        .image = pic_bgr,
        .image_h = pic_h,
        .image_w = pic_w,
        .block_w = 100,
        .block_h = 100,
        .block_x = 0,
        .block_y = 0,
        .select = GAME_SEL_NONE
    };

    game.nr_block_h = game.image_w / game.block_w;
    game.nr_block_v = game.image_h / game.block_h;

    // TODO malloc ;-)
    int nr_block = game.nr_block_h * game.nr_block_v;
    int block[nr_block];

    // TODO more random
    for (int i = 0; i < nr_block; i++) {
        block[i] = nr_block - 1 - i;
    }

    game.block = block;

    for (;;) {
        update_frame(&game);

        if (check_valid(&game)) {
            break;
        }

        update_game_state(&game);
    }

    LOG("Success");

    init_video_buffer();
    for (int y = 0; y < pic_h; y++) {
        for (int x = 0; x < pic_w; x++) {
            draw_pixel(x, y, get_color(pic_bgr, x, y, pic_w, pic_h));
        }
    }
    update_screen();
}
