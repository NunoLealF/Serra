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
# -pie: Make a position-independent executable.
# -Wl,--no-dynamic-linker: Tell the linker to provide its own startup code (no dynamic linker needed).

# [Compiler flags]
# -funsigned-char: Use unsigned chars for characters.
# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.
# -fpie: Compile our code into a position-independent executable.
# -std=c23: Use the regular C23 standard (GCC 15+, Clang 20+)
# -maccumulate-outgoing-args: Necessary for the EFI/Microsoft ABI on GCC.
# -march=x86-64: Make sure only base x64 instructions are used.
# -mmmx, -msse, -msse2: Make sure to *enable* MMX, SSE and SSE2 instructions.
# -mno-red-zone: Disable the System V ABI 'red zone'; this is necessary for now.
# -O3: Optimize the resulting file/binary for *speed*.
# -Wall: Enable all warnings.
# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)
# -Wpedantic: Be pedantic about the C standard / any possible mistakes

LDFLAGS = -nostdlib -mno-red-zone -pie -Wl,--no-dynamic-linker
CFLAGS = -std=c23 -funsigned-char -ffreestanding -fpie -maccumulate-outgoing-args -march=x86-64 -mmmx -msse -msse2 -mno-red-zone -O3 -Wall -Wextra -Wpedantic

# (Include the build configuration file, from the root directory)

CONFIG := ../makefile.config
include ${CONFIG}


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

	@-rm -f Kernel/Constructors/Firmware/*.o
	@-rm -f Kernel/Constructors/System/*.o

	@-rm -f Kernel/Firmware/*.o
	@-rm -f Kernel/Firmware/*.o

	@-rm -f Kernel/Graphics/*.o
	@-rm -f Kernel/Graphics/Console/*.o
	@-rm -f Kernel/Graphics/Fonts/*.o

	@-rm -f Kernel/Libraries/*.o
	@-rm -f Kernel/Libraries/Memory/*.o
	@-rm -f Kernel/Libraries/Memory/x64/*.o

	@-rm -f Kernel/System/x64/*.o

Dump:
	@$(OBJD) -S Kernel/Kernel.elf


# Lowercase names

all: All
compile: Compile
clean: Clean
dump: Dump



# [The files themselves]

# (Entrypoint and core files)

Kernel/Entry.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -ffunction-sections -c Kernel/Entry.c -o Kernel/Entry.o

Kernel/Core.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Core.c -o Kernel/Core.o

# (Constructors - these set up the kernel libraries)

Kernel/Constructors/Firmware/Efi.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Constructors/Firmware/Efi.c -o Kernel/Constructors/Firmware/Efi.o

Kernel/Constructors/System/x64.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Constructors/System/x64.c -o Kernel/Constructors/System/x64.o

# (Firmware-specific files/libraries)

Kernel/Firmware/Efi.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Firmware/Efi.c -o Kernel/Firmware/Efi.o

# (Kernel libraries)

Kernel/Graphics/Graphics.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Graphics.c -o Kernel/Graphics/Graphics.o

Kernel/Graphics/Console/Console.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Console/Console.c -o Kernel/Graphics/Console/Console.o

Kernel/Graphics/Console/Efi.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Console/Efi.c -o Kernel/Graphics/Console/Efi.o

Kernel/Graphics/Console/Exceptions.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Console/Exceptions.c -o Kernel/Graphics/Console/Exceptions.o

Kernel/Graphics/Console/Format.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Console/Format.c -o Kernel/Graphics/Console/Format.o

Kernel/Graphics/Console/Graphical.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -Wno-sign-compare -c Kernel/Graphics/Console/Graphical.c -o Kernel/Graphics/Console/Graphical.o

Kernel/Graphics/Console/Vga.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Console/Vga.c -o Kernel/Graphics/Console/Vga.o

Kernel/Graphics/Fonts/Bitmap.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Graphics/Fonts/Bitmap.c -o Kernel/Graphics/Fonts/Bitmap.o

Kernel/Libraries/Memory/Memory.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Libraries/Memory/Memory.c -o Kernel/Libraries/Memory/Memory.o

Kernel/Libraries/Memory/String.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/Libraries/Memory/String.c -o Kernel/Libraries/Memory/String.o

Kernel/Libraries/Memory/x64/Memcpy.o:
	@echo "Building $@"
	@$(AS) Kernel/Libraries/Memory/x64/Memcpy.asm -f elf64 -o Kernel/Libraries/Memory/x64/Memcpy.o

Kernel/Libraries/Memory/x64/Memset.o:
	@echo "Building $@"
	@$(AS) Kernel/Libraries/Memory/x64/Memset.asm -f elf64 -o Kernel/Libraries/Memory/x64/Memset.o

# (System/platform-specific files)

Kernel/System/x64/Cpu.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Kernel/System/x64/Cpu.c -o Kernel/System/x64/Cpu.o

# Link everything into one .elf file

Kernel/Kernel.elf: Kernel/Entry.o Kernel/Core.o Kernel/Constructors/Firmware/Efi.o Kernel/Constructors/System/x64.o Kernel/Firmware/Efi.o Kernel/Graphics/Graphics.o Kernel/Graphics/Console/Console.o Kernel/Graphics/Console/Efi.o Kernel/Graphics/Console/Exceptions.o Kernel/Graphics/Console/Format.o Kernel/Graphics/Console/Graphical.o Kernel/Graphics/Console/Vga.o Kernel/Graphics/Fonts/Bitmap.o Kernel/Libraries/Memory/Memory.o Kernel/Libraries/Memory/x64/Memcpy.o Kernel/Libraries/Memory/x64/Memset.o Kernel/Libraries/Memory/String.o Kernel/System/x64/Cpu.o
	@echo "Building $@"
	@$(CC) -T Linker.ld $(LDFLAGS) -o Kernel/Kernel.elf -ffreestanding -nolibc -nostdlib -lgcc $^
