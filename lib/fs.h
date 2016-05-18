#ifndef _FS_H_
#define _FS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#pragma pack(0)
struct dirent {
    char fs_name[24];
    uint32_t fs_file_size;
    uint32_t fs_inode_off;
};

struct file {
    union {
        uint8_t data[512];
        uint32_t fs_inode_off[(512 - 32) / sizeof(uint32_t)];
        struct {
            char fs_dir_name[32];
            struct dirent fs_files[(512 - 32) / sizeof(struct dirent)];
        };
    };
};

// 1b -> 512B == 0.5KB
// 1B == 8b -> 4096B == 4KB
// 512B == 0.5K * 4KB = 2MB
struct bitmap {
    unsigned char bm_data[512 * 256];
};
#pragma pack()

void putbyte_chomp(unsigned char byte);
void putbyte(unsigned char byte);
void putbm(struct bitmap *bm);
unsigned fs_get_bit(const struct bitmap *bm, unsigned offset);
void fs_set_bit(struct bitmap *bm, unsigned offset, unsigned val);
unsigned fs_get_blk_sz(unsigned sz);
unsigned fs_alloc_blk(struct bitmap *bm);
uint32_t fs_alloc_file_in_dir(struct bitmap *bm, struct file *dir, const char *basename, uint32_t filesz);

#endif // _FS_H_
