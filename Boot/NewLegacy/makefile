# Copyright (C) 2024 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an i686-elf-gcc toolchain and nasm

AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
OBJC = i686-elf-objcopy
OBJD = i686-elf-objdump


# Compiler flags:

# -ffunction-sections: Split up every function into a separate section in the object file.
# You want to place the main function in the beginning, so this is needed.

# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.

# -std=c11: Use the regular C11 standard

# -Wall: Enable all warnings.

# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)

# -Wpedantic: Be pedantic about the C standard / any possible mistakes

# -funsigned-char: Use unsigned chars for characters.

# -Os: Optimize for size (and hope the code doesn't break).

LDFLAGS = -T Legacy/Linker.ld -Wl,--no-warn-rwx-segments
CFLAGS = -std=c11 -ffunction-sections -funsigned-char -ffreestanding -Wall -Wextra -Wpedantic


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Clean Dump Run RunInt all dump clean run runint


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Legacy/Bootsector.bin Legacy/Bootloader.o Legacy/Bootloader.bin Boot.bin

Clean:
	@echo "Cleaning leftover files (*.o, *.elf, *.bin).."

	@-rm -f Bootsector/*.o
	@-rm -f Bootsector/*.bin

	@-rm -f Init/*.o
	@-rm -f Init/*.elf
	@-rm -f Init/*.bin

	@-rm -f Core/*.o
	@-rm -f Core/*.elf
	@-rm -f Core/*.bin

	@-rm -f Shared/Rm/*.o
	@-rm -f Shared/Rm/*.elf
	@-rm -f Shared/Rm/*.bin

	@-rm -f *.bin

Dump:
	$(OBJD) -S Legacy/Bootloader.elf

Dumpbin:
	$(OBJD) -Mintel,i686 -b binary -m i386 --adjust-vma=0x7E00 -D Legacy/Bootloader.bin

Run:
	@echo "Launching Qemu.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Boot.bin,format=raw

RunInt:
	@echo "Launching Qemu with -d int.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Boot.bin,format=raw -d int

# Lowercase names

all: All
clean: Clean
dump: Dump
dumpbin: Dumpbin
run: Run
runint: RunInt


# Files themselves

# (First-stage bootloader, Legacy/Bootsector)

Bootsector/Bootsector.bin:
	@echo "Building $@"
	@$(AS) Bootsector/Bootsector.asm -f bin -o Bootsector/Bootsector.bin

# (Second-stage bootloader, Legacy/Init)

Init/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Bootloader.c -o Init/Bootloader.o

# (Third-stage bootloader, Legacy/Core)

# (Shared code, Legacy/Shared)

Shared/Rm/Rm.bin:
	@echo "Building $@"
	@$(AS) Shared/Rm/Rm.asm -f bin -o Shared/Rm/Rm.bin

Shared/Rm/RmWrapper.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Rm/RmWrapper.c -o Shared/Rm/RmWrapper.o

# Link the stages into .bin

Init/Init.bin: Init/Bootloader.o
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -o Init/Init.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Init/Init.elf Init/Init.bin

Core/Core.bin:
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -o Core/Core.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Core/Core.elf Core/Core.bin

# Mix all of the different stages together (128 sectors, so 64KiB img)

Legacy.bin: Bootsector/Bootsector.bin Init/Init.bin Shared/Rm/Rm.bin Core/Core.bin

	@echo "Building $@"
	@dd if=/dev/zero of=Boot.bin bs=512 count=128 status=none

	@dd if=Bootsector/Bootsector.bin of=Boot.bin conv=notrunc bs=512 count=1 seek=0 status=none
	@dd if=Init/Init.bin of=Boot.bin conv=notrunc bs=512 count=16 seek=7 status=none
	@dd if=Shared/Rm/Rm.bin of=Boot.bin conv=notrunc bs=512 count=8 seek=23 status=none
	@dd if=Core/Core.bin of=Boot.bin conv=notrunc bs=512 count=64 seek=63 status=none
