# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an x86_64-w64-mingw32-gcc toolchain that supports the C23 standard.
# (If you're on Ubuntu 24.04+, you can use apt; otherwise, you *must* cross-compile it)

CC = x86_64-w64-mingw32-gcc
LD = x86_64-w64-mingw32-ld
OBJC = x86_64-w64-mingw32-objcopy
OBJD = x86_64-w64-mingw32-objdump


# [Linker flags]
# -e Bootloader: Our function entrypoint should be the Bootloader function.
# -nostdlib: Don't use or link any standard libraries (except for libgcc).
# -static-pie: Make a *static* position-independent executable (that can relocate itself)
# -Wl,--subsystem,10: Set the subsystem type in the PE header to 10 (UEFI executable).

# [Compiler flags]
# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.
# -fpie: Compile our code into a position-independent executable.
# -maccumulate-outgoing-args: Necessary for the EFI/Microsoft ABI on GCC.
# -std=c2x: Use the regular C2x/C23 standard.
# -O3: Optimize the resulting file/binary for speed.
# -Wall: Enable all warnings.
# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)
# -Wpedantic: Be pedantic about the C standard / any possible mistakes

LDFLAGS = -e Bootloader -nostdlib -static-pie -Wl,--subsystem,10
CFLAGS = -ffreestanding -fpie -maccumulate-outgoing-args -O0 -std=c2x -Wall -Wextra -Wpedantic


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Compile Clean Dump all compile dump clean


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile: Bootx64.efi

Clean:
	@echo "\033[0;2m""Cleaning leftover files (*.o, *.elf, *.bin).." "\033[0m"

	@-rm -f *.o
	@-rm -f *.efi

Dump:
	@$(OBJD) -S Bootx64.efi


# Lowercase names

all: All
compile: Compile
clean: Clean
dump: Dump



# [The files themselves]

Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -ffunction-sections -c Bootloader.c -o Bootloader.o

# [Link everything into one .efi (PE executable) file]

Bootx64.efi: Bootloader.o
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -o Bootx64.efi -ffreestanding -nolibc -nostdlib -lgcc $^
