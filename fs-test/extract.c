#include "fs.h"

int main(int argc, char *argv[])
{
    struct bitmap bitmap = {};
    struct file root = {};
    FILE *disk = fopen(argv[1], "rb");
    FILE *dump = fopen(argv[2], "wb");

    fread(&bitmap, sizeof(bitmap), 1, disk);
    fread(&root, sizeof(root), 1, disk);

    for (int i = 0; i < sizeof(root.fs_files) / sizeof(root.fs_files[0]); i++) {
        if (root.fs_files[i].fs_inode_off != 0) {
            printf("read file %s\n", root.fs_files[i].fs_name);
            uint32_t inode_off = root.fs_files[i].fs_inode_off;
            struct file file;
            fseek(disk, inode_off * 512, SEEK_SET);
            fread(&file, sizeof(file), 1, disk);
            uint32_t filesz = root.fs_files[i].fs_file_size;
            printf("filesz %u\n", filesz);
            for (int j = 0; file.fs_inode_off[j] != 0xcccccccc; j++) {
                printf("The %dth block is at %d\n",
                        j, file.fs_inode_off[j] * 512);
                fseek(disk, file.fs_inode_off[j] * 512, SEEK_SET);
                uint8_t buffer[512];
                fread(buffer, sizeof(buffer), 1, disk);
                fwrite(buffer, filesz < 512 ? filesz : sizeof(buffer), 1, dump);
                filesz -= 512;
            }
        }
    }
}
