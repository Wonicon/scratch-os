#include "fs.h"

void putbyte_chomp(unsigned char byte)
{
    for (int i = 0; i < 8; i++) {
        putchar((byte & 0x1) ? '1' : '0');
        byte >>= 1;
    }
}

void putbyte(unsigned char byte)
{
    putbyte_chomp(byte);
    putchar('\n');
}

void putbm(struct bitmap *bm)
{
    for (unsigned i = 0; i < sizeof(*bm); i++) {
        putbyte_chomp(bm->bm_data[i]);
    }
    putchar('\n');
}

unsigned fs_get_bit(const struct bitmap *bm, unsigned offset)
{
    return (bm->bm_data[offset >> 3] >> (offset & 0x7)) & 0x1;
}

void fs_set_bit(struct bitmap *bm, unsigned offset, unsigned val)
{
    unsigned char map = (val & 0x1) << (offset & 0x7);
    unsigned char mask = ~(1 << (offset & 0x7));
    unsigned char old = bm->bm_data[offset >> 3];
    bm->bm_data[offset >> 3] = (old & mask) | map;
}

unsigned fs_get_blk_sz(unsigned sz)
{
    return ((sz - 1) / 512) * 512;
}

// Find the first available block in the bitmap
unsigned fs_alloc_blk(struct bitmap *bm)
{
    int idx = -1;
    for (unsigned i = 0; i < sizeof(*bm); i++) {
        unsigned char mask = ~(bm->bm_data[i]);
        if (mask) {  // This byte of bitmap is not all one
            mask = (mask & (-mask));  // Extract the first 1, where is the first 0 in bitmap
            while (mask) {
                idx++;
                mask >>= 1;
            }
            idx = (i << 3) | idx;
            fs_set_bit(bm, idx, 1);
            break;
        }
    }
    return idx;
}

// On success return inode block offset
// On error return 0xffffffff
uint32_t fs_alloc_file_in_dir(struct bitmap *bm, struct file *dir, const char *basename, uint32_t filesz)
{
    printf("Try to alloc file `%s' in dir `%s'\n", basename, dir->fs_dir_name);
    for (int i = 0; i < sizeof(dir->fs_files) / sizeof(*dir->fs_files); i++) {
        if (dir->fs_files[i].fs_inode_off != (~0)) {
            printf("Alloc file `%s' in dir `%s' slot %d\n",
                    basename, dir->fs_dir_name, i);
            strcpy(dir->fs_files[i].fs_name, basename);
            dir->fs_files[i].fs_inode_off = fs_alloc_blk(bm);
            dir->fs_files[i].fs_file_size = filesz;
            return dir->fs_files[i].fs_inode_off;
        }
    }
    return (~0);
}
