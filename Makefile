BOOT   := boot.bin
KERNEL := kernel.bin
IMAGE  := disk.bin

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386
GDB     := gdb

CFLAGS := -Wall -Werror -Wfatal-errors -MD
CFLAGS += -std=gnu11 -m32 -c
CFLAGS += -I .
CFLAGS += -O0
CFLAGS += -fno-builtin
CFLAGS += -fno-stack-protector
CFLAGS += -ggdb3

QEMU_OPTIONS := -serial stdio -d int
QEMU_OPTIONS += -monitor telnet:127.0.0.1:1111,server,nowait

QEMU_DEBUG_OPTIONS := -S
QEMU_DEBUG_OPTIONS += -s

GDB_OPTIONS := -ex "target remote 127.0.0.1:1234"
GDB_OPTIONS += -ex "symbol $(KERNEL)"

OBJ_DIR        := obj
LIB_DIR        := lib
BOOT_DIR       := boot
KERNEL_DIR     := kernel
OBJ_LIB_DIR    := $(OBJ_DIR)/$(LIB_DIR)
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)

LD_SCRIPT := $(shell find $(KERNEL_DIR) -name "*.ld")

LIB_C := $(wildcard $(LIB_DIR)/*.c)
LIB_O := $(LIB_C:%.c=$(OBJ_DIR)/%.o)

BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_S := $(shell find $(KERNEL_DIR) -name "*.S")
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o)
KERNEL_O += $(KERNEL_S:%.S=$(OBJ_DIR)/%.o)

$(IMAGE): $(BOOT) $(KERNEL)
	@$(DD) if=/dev/zero of=$(IMAGE) count=10000         > /dev/null # 准备磁盘文件
	@$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc          > /dev/null # 填充 boot loader
	@$(DD) if=$(KERNEL) of=$(IMAGE) seek=1 conv=notrunc > /dev/null # 填充 kernel, 跨过 mbr

$(BOOT): $(BOOT_O)
	$(LD) -e start -Ttext=0x7C00 -m elf_i386 -nostdlib -o $@.out $^
	$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@rm $@.out
	ruby mbr.rb $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.S
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(KERNEL): $(LD_SCRIPT)
$(KERNEL): $(KERNEL_O) $(LIB_O)
	$(LD) -m elf_i386 -T $(LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
	@mkdir -p $(OBJ_LIB_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.[cS]
	mkdir -p $(OBJ_DIR)/$(dir $<)
	$(CC) $(CFLAGS) $< -o $@

DEPS := $(shell find -name "*.d")
-include $(DEPS)

.PHONY: qemu debug gdb clean

qemu: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) $(IMAGE)

# Faster, but not suitable for debugging
qemu-kvm: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) --enable-kvm $(IMAGE)

debug: $(IMAGE)
	$(QEMU) $(QEMU_DEBUG_OPTIONS) $(QEMU_OPTIONS) $(IMAGE)

gdb:
	$(GDB) $(GDB_OPTIONS)

gen-disk: lib/fs.c lib/fs.h fs-test/format.c
	$(CC) -O0 -g -I lib -o $@ lib/fs.c fs-test/format.c

extract: lib/fs.c lib/fs.h fs-test/extract.c
	$(CC) -O0 -g -I lib -o $@ lib/fs.c fs-test/extract.c

clean:
	@rm -rf $(OBJ_DIR) 2> /dev/null
	@rm -rf $(BOOT)    2> /dev/null
	@rm -rf $(KERNEL)  2> /dev/null
	@rm -rf $(IMAGE)   2> /dev/null
