# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an x86_64-elf-gcc toolchain and nasm.
# (Libgcc should be compiled without the red zone: https://wiki.osdev.org/Libgcc_without_red_zone)

AS = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJC = x86_64-elf-objcopy
OBJD = x86_64-elf-objdump


# [Linker flags]
# -nostdlib: Don't use or link any standard libraries (except for libgcc).
# -mno-red-zone: Disable the System V ABI 'red zone'; this is necessary for now.

# [Compiler flags]
# -funsigned-char: Use unsigned chars for characters.
# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.
# -std=c11: Use the regular C11 standard.
# -mcmodel=kernel: Assume the kernel runs in the last 2GiB of address space
# -mno-red-zone: Disable the System V ABI 'red zone'; this is necessary for now.
# -O2: Optimize the resulting file/binary for speed.
# -Wall: Enable all warnings.
# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)
# -Wpedantic: Be pedantic about the C standard / any possible mistakes

LDFLAGS = -nostdlib -mcmodel=kernel -mno-red-zone
CFLAGS = -std=c11 -funsigned-char -ffreestanding -mcmodel=kernel -mno-red-zone -O2 -Wall -Wextra -Wpedantic


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Compile Clean Dump all compile dump clean


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile: Kernel/Kernel.elf

Clean:
	@echo "\033[0;2m""Cleaning leftover files (*.o, *.elf, *.bin).." "\033[0m"

	@-rm -f Kernel/*.o
	@-rm -f Kernel/*.elf
	@-rm -f Kernel/*.bin

Dump:
	@$(OBJD) -S Kernel.elf


# Lowercase names

all: All
compile: Compile
clean: Clean
dump: Dump



# [The files themselves]

Kernel/Entry.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -mno-mmx -mno-sse -mno-sse2 -ffunction-sections -c Kernel/Entry.c -o Kernel/Entry.o

# Link everything into one .elf file

Kernel/Kernel.elf: Kernel/Entry.o
	@echo "Building $@"
	@$(CC) -T Linker.ld $(LDFLAGS) -o Kernel/Kernel.elf -ffreestanding -nolibc -nostdlib -lgcc $^
