/**
 * 废弃代码, 纯作参考
 */

#include <inc/mmu.h>

/**
 * 文字模式字符显式属性格式
 * 参见 https://en.wikipedia.org/wiki/VGA-compatible_text_mode
 * fg 和 bg 是调色版的下标，对应颜色参见 https://en.wikipedia.org/wiki/Video_Graphics_Array#Color_palette
 * blink 对应背景高亮，实际上与 bg 一起组成 16 色的下标
 */
typedef union {
    struct {
        uint8_t fg    : 4;
        uint8_t bg    : 3;
        uint8_t blink : 1;
    };
    uint8_t value;
} Attribute;


void hello_world(void)
{
    /**
     * 代码参考自 http://wiki.0xffffff.org/posts/hurlex-3.html
     * 离线版文档 http://hurlex.0xffffff.org/
     */
    uint8_t *input = (uint8_t *)0xB8000;
    const char *str = "Hello World";

    Attribute attr = {
        .fg    = 0x2,  // green
        .bg    = 0x5,  // magneta
        .blink = 0x0,  // normal
    };

    while (*str) {
        *input++ = *str++;
        *input++ = attr.value;
    }
}

