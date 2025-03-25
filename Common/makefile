# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an x86_64-w64-mingw32-gcc toolchain and nasm.

AS = nasm
CC = x86_64-w64-mingw32-gcc
LD = x86_64-w64-mingw32-ld
OBJC = x86_64-w64-mingw32-objcopy
OBJD = x86_64-w64-mingw32-objdump


# [Linker flags]
# -nostdlib: Don't use/link any standard libraries!

# [Compiler flags]
# -ffunction-sections: Split up every function into a separate section in the object file.
# (You want to place the main function in the beginning, so this is needed.)
# -funsigned-char: Use unsigned chars for characters.
# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.
# -std=c11: Use the regular C11 standard
# -O2: Optimize the resulting file/binary for speed.
# -mcmodel=kernel: (TODO - run in the last 2GiB of memory)
# -mno-red-zone: (TODO - necessary for 64-bit)
# -Wall: Enable all warnings.
# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)
# -Wpedantic: Be pedantic about the C standard / any possible mistakes

LDFLAGS = -nostdlib
CFLAGS = -std=c11 -ffunction-sections -funsigned-char -ffreestanding -mcmodel=kernel -mno-red-zone -O2 -Wall -Wextra -Wpedantic


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Compile Clean Dump Dumpbin all compile dump dumpbin clean


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile:
	@echo "\033[0;2m""// (TODO, there's nothing here yet..)" "\033[0m"

Clean:
	@echo "\033[0;2m""Cleaning leftover files (*.o, *.elf, *.bin).." "\033[0m"


# (...)

Dump:
	# @$(OBJD) -S Core/Serra.elf

Dumpbin:
	# @$(OBJD) -Mintel,i686 -b binary -m i386 --adjust-vma=0x7E00 -D Legacy.bin

# Lowercase names

all: All
compile: Compile
clean: Clean
dump: Dump
dumpbin: Dumpbin



# [The files themselves]

# (The stub thing we have to jump to first - TODO)

# (The kernel itself - TODO)
