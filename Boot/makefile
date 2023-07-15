# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# This is a makefile. Todo!
# Also, TODO - This is supposed to eventually be for both legacy and UEFI boot folders
# For now I'm just doing legacy but you'll need a way to merge them in the future


# You'll need an i686-elf-gcc toolchain and nasm

AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
OBJ = i686-elf-objcopy


# Compiler flags:

# -ffunction-sections: Split up every function into a separate section in the object file. You want to place the main function
# in the beginning, so this is needed.

# -ffreestanding: This is a freestanding environment lol. Also implies fno-builtin

# -std=c11: Use the regular C11 standard

# -Wall: Enable all warnings.

# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)

# -Wpedantic: Be pedantic about the C standard / any possible mistakes

# -funsigned-char: Use unsigned chars for characters.

# -Os: Optimize for size (and hope the code doesn't break).

LDFLAGS = -T Legacy/Linker.ld
CFLAGS = -std=c11 -ffunction-sections -funsigned-char -ffreestanding -Wall -Wextra -Wpedantic


# I'm just gonna steal this from Ribeira (another project I also did):

# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Clean Run all clean run


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Legacy/Bootsector.bin Legacy/Bootloader.o Legacy/Bootloader.bin Boot.bin

Clean:
	@echo "Cleaning leftover files (*.o, *.elf, *.bin).."

	@-rm -f Legacy/*.o
	@-rm -f Legacy/Memory/*.o

	@-rm -f Legacy/*.elf
	@-rm -f Legacy/*.bin

	@-rm -f *.bin

Run:
	@echo "Launching Qemu.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Boot.bin,format=raw

# Lowercase names

all: All
clean: Clean
run: Run


# Files themselves
# (Legacy/Bootloader)

Legacy/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Legacy/Bootloader.c -o Legacy/Bootloader.o

# (Legacy/Memory)

Legacy/Memory/A20.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Legacy/Memory/A20.c -o Legacy/Memory/A20.o


# Link the stages into .bin

Legacy/Bootsector.bin:
	@echo "Building $@"
	@$(AS) Legacy/Bootsector.asm -f bin -o Legacy/Bootsector.bin

Legacy/Bootloader.bin: Legacy/Bootloader.o Legacy/Memory/A20.o
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -o Legacy/Bootloader.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@objcopy -O binary Legacy/Bootloader.elf Legacy/Bootloader.bin

# Mix the two together (64 sectors, so 32KiB img)

Boot.bin: Legacy/Bootsector.bin Legacy/Bootloader.bin
	@echo "Building $@"
	@dd if=/dev/zero of=Boot.bin bs=512 count=64 status=none
	@dd if=Legacy/Bootsector.bin of=Boot.bin conv=notrunc bs=512 count=1 status=none
	@dd if=Legacy/Bootloader.bin of=Boot.bin conv=notrunc bs=512 count=63 seek=1 status=none
