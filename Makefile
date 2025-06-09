# Compiler and linker settings
ASM=nasm
CC="/mnt/c/i686-elf-tools-windows/bin/i686-elf-gcc.exe"
LD="/mnt/c/i686-elf-tools-windows/bin/i686-elf-ld.exe"

# Flags
ASMFLAGS=-f elf32
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -Iinclude
LDFLAGS=-m elf_i386 -T linker.ld -nostdlib

# Directories
SRC_DIR=src
BUILD_DIR=build
INCLUDE_DIR=include

# Source files
ASM_SOURCES=$(wildcard $(SRC_DIR)/*.asm)
C_SOURCES=$(wildcard $(SRC_DIR)/*.c)

# Object files (in build/)
ASM_OBJECTS=$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
C_OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
OBJECTS=$(ASM_OBJECTS) $(C_OBJECTS)

# Output files
KERNEL=kernel.bin
ISO=geeos.iso

# Default target
all: $(KERNEL)

# Link kernel
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compile assembly to build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $<

# Compile C to build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Build ISO
iso: $(KERNEL)
	mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/
	cp boot/grub/grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

# Clean
clean:
	rm -f $(BUILD_DIR)/*.o $(KERNEL) $(ISO)
	rm -rf $(BUILD_DIR) isodir

# Run
run: iso
	qemu-system-i386 -cdrom $(ISO)

.PHONY: all iso clean run
