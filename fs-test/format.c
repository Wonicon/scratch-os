#include "fs.h"

void fs_write_file(struct bitmap *bm, struct file *dir, const char *filename, FILE *disk_fp)
{
    FILE *fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    uint32_t inode_off = fs_alloc_file_in_dir(bm, dir, filename, ftell(fp));
    fseek(fp, 0, SEEK_SET);
    struct file file = {};
    memset(&file, 0xcc, sizeof(file));
    
    int idx = 0;
    uint8_t buffer[512];
    memset(buffer, 0, sizeof(buffer));
    while (fread(buffer, 1, sizeof(buffer), fp)) {
        uint32_t off = fs_alloc_blk(bm);
        file.fs_inode_off[idx++] = off;
        fseek(disk_fp, off * 512, SEEK_SET);
        fwrite(buffer, sizeof(buffer), 1, disk_fp);
        memset(buffer, 0, sizeof(buffer));
    }

    printf("Totally %d blocks of data have been written.\n", idx);
    fclose(fp);

    fseek(disk_fp, inode_off * 512, SEEK_SET);
    fwrite(&file, sizeof(file), 1, disk_fp);
}

int main(int argc, char *argv[])
{
    printf("sect size %lu\n", sizeof(struct file));
    printf("bitmap size %lu\n", sizeof(struct bitmap));
    printf("disk size %lu\n", sizeof(struct bitmap) * 8 * 512);

    struct bitmap bm = {};
    size_t size = sizeof(bm);
    size = (((size - 1) / 512) + 1) * 512;
    unsigned idx = 0;
    while (size) {
        fs_set_bit(&bm, idx++, 1);
        size -= 512;
    }

    struct file dir;
    strcpy(dir.fs_dir_name, "hello");
    unsigned off = fs_alloc_blk(&bm);
    printf("wow, I get a block %d\n", off);
    
    FILE *fp = fopen("a.disk", "wb");

    fs_write_file(&bm, &dir, argv[1], fp);

    fseek(fp, 0, SEEK_SET);
    fwrite(&bm, sizeof(bm), 1, fp);
    fwrite(&dir, sizeof(dir), 1, fp);
    fclose(fp);
}
