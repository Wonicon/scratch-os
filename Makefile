BOOT   := boot.bin
KERNEL := kernel.bin
IMAGE  := disk.bin

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386

CFLAGS := -Wall -Werror -Wfatal-errors
CFLAGS += -std=gnu11 -m32 -c -MD
CFLAGS += -I .
CFLAGS += -O3
CFLAGS += -fno-builtin

QEMU_OPTIONS := -monitor telnet:127.0.0.1:1234,server,nowait

OBJ_DIR        := obj
BOOT_DIR       := boot
KERNEL_DIR     := kernel
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)

BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(wildcard $(KERNEL_DIR)/*.c)

$(IMAGE): $(BOOT) $(KERNEL)
	$(DD) if=/dev/zero of=$(IMAGE) count=10000          # 准备磁盘文件
	$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc           # 填充 boot loader
	$(DD) if=$(KERNEL) of=$(IMAGE) seek=1 conv=notrunc  # 填充 kernel, 跨过 mbr

$(BOOT): $(BOOT_O)
	$(LD) -e start -Ttext=0x7C00 -m elf_i386 -o $@.out $^
	$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@rm $@.out
	ruby mbr.rb $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.S
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(KERNEL): $(KERNEL_O)
	$(LD) -e main -Ttext=0x100000 -m elf_i386 -o $@ $^

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(OBJ_KERNEL_DIR)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: qemu clean

qemu: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) $(IMAGE)

clean:
	@rm -rf $(OBJ_DIR) 2> /dev/null
	@rm -rf $(BOOT)    2> /dev/null
	@rm -rf $(KERNEL)  2> /dev/null
	@rm -rf $(IMAGE)   2> /dev/null
