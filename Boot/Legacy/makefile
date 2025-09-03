# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an i686-elf toolchain and nasm, as well as qemu if you want
# to emulate it.

AS = nasm
CC = i686-elf-gcc # (Can be replaced with `clang`)
LD = i686-elf-ld # (Can be replaced with `lld`)
OBJC = i686-elf-objcopy
OBJD = i686-elf-objdump
QEMU = qemu-system-x86_64


# [Linker flags]

# -m32: Make sure we're only linking 32-bit code.
# -Wl,-no-pie: Make it clear we're not generating a position-independent binary.

# [Compiler flags]

# -ffunction-sections: Split up every function into a separate section in the object file.
# (You want to place the main function in the beginning, so this is needed.)
# -ffreestanding: This is a freestanding environment. Also, implies fno-builtin.
# -fno-pic: Make it clear that we're not generating position-independent code.
# -funsigned-char: Use unsigned characters by default.
# -std=c23: Use the regular C23 standard (GCC 15+, Clang 20+)
# -m32: Make sure we're only compiling 32-bit code.
# -mno-mmx, -mno-sse, -mno-sse2: Make sure to *disable* MMX, SSE and SSE2 instructions.
# -Os: Optimize the resulting file/binary for *space*. (O3 may cause problems!)
# -Wall: Enable all warnings.
# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)
# -Wpedantic: Be pedantic about the C standard / any possible mistakes

LDFLAGS = -m32 -Wl,--no-pie
CFLAGS = -m32 -std=c23 -ffunction-sections -ffreestanding -fno-pic -funsigned-char -mno-mmx -mno-sse -mno-sse2 -Os -Wall -Wextra -Wpedantic

# [Compiler-specific options]

# -Wno-discarded-qualifiers (GCC): Don't warn about discarding qualifiers
# -Wl,--no-warn-rwx-segments (GCC): Don't warn about mixing data and executable sections

# -Wno-ignored-qualifiers (Clang): Don't warn about ignoring qualifiers
# --target=i686-pc-none-elf (Clang): Specify the target being used
# -fuse-ld=$(LD) (Clang): Use the specified linker (this is usually LLD)

ifneq ('', '$(findstring gcc,$(CC))')
  CFLAGS += -Wno-discarded-qualifiers
	LDFLAGS += -Wl,--no-warn-rwx-segments
else ifneq ('', '$(findstring clang,$(CC))')
	LDFLAGS += -fuse-ld=$(LD)
	CFLAGS += --target=i686-pc-none-elf -Wno-ignored-qualifiers
endif


# (Include the build configuration file, from the root directory)

CONFIG := ../../makefile.config
include ${CONFIG}


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Clean Compile Dump Dumpbin all clean compile dump dumpbin


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile: Bootsector/Mbr.bin Bootsector/Bootsector.bin Stage2/Stage2.bin Shared/Rm/Rm.bin Stage3/Stage3.bin Bootx32.bin

Clean:
	@echo "\033[0;2m""Cleaning leftover files (*.o, *.elf, *.bin).." "\033[0m"

	@-rm -f Bootsector/*.o
	@-rm -f Bootsector/*.bin

	@-rm -f Stage2/*.o
	@-rm -f Stage2/*.elf
	@-rm -f Stage2/*.bin

	@-rm -f Stage3/*.o
	@-rm -f Stage3/*.elf
	@-rm -f Stage3/*.bin

	@-rm -f Stage3/Cpu/*.o
	@-rm -f Stage3/Graphics/*.o
	@-rm -f Stage3/Int/*.o
	@-rm -f Stage3/Memory/A20/*.o
	@-rm -f Stage3/Memory/Mmap/*.o
	@-rm -f Stage3/Memory/Paging/*.o

	@-rm -f Shared/Disk/*.o
	@-rm -f Shared/Graphics/*.o
	@-rm -f Shared/Memory/*.o
	@-rm -f Shared/Rm/*.o
	@-rm -f Shared/Rm/*.bin


	@-rm -f *.bin


# (...)

Dump:
	@$(OBJD) -S Stage3/Stage3.elf

Dumpbin:
	@$(OBJD) -Mintel,i686 -b binary -m i386 --adjust-vma=0x7E00 -D Stage2/Stage2.bin

# Lowercase names

all: All
clean: Clean
compile: Compile
dump: Dump
dumpbin: Dumpbin



# [The files themselves]

# (MBR and first-stage bootloader (VBR), Legacy/Bootsector)

Bootsector/Mbr.bin:
	@echo "Building $@"
	@$(AS) Bootsector/Mbr.asm -f bin -o Bootsector/Mbr.bin

Bootsector/Bootsector.bin:
	@echo "Building $@"
	@$(AS) Bootsector/Bootsector.asm -f bin -o Bootsector/Bootsector.bin


# (Second-stage bootloader, Legacy/Stage2)

Stage2/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage2/Bootloader.c -o Stage2/Bootloader.o


# (Third-stage bootloader, Legacy/Stage3)

Stage3/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Bootloader.c -o Stage3/Bootloader.o

Stage3/Elf.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Elf.c -o Stage3/Elf.o

Stage3/Stub.o:
	@echo "Building $@"
	@$(AS) -w-zext-reloc Stage3/Stub.asm -f elf -o Stage3/Stub.o

Stage3/Cpu/Cpu.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -Wno-array-bounds -c Stage3/Cpu/Cpu.c -o Stage3/Cpu/Cpu.o

Stage3/Memory/A20/A20.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Memory/A20/A20.s -o Stage3/Memory/A20/A20.o

Stage3/Memory/A20/A20_Wrapper.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Memory/A20/A20_Wrapper.c -o Stage3/Memory/A20/A20_Wrapper.o

Stage3/Memory/Mmap/Mmap.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Memory/Mmap/Mmap.c -o Stage3/Memory/Mmap/Mmap.o

Stage3/Memory/Paging/Paging.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Memory/Paging/Paging.c -o Stage3/Memory/Paging/Paging.o

Stage3/Graphics/Exceptions.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Graphics/Exceptions.c -o Stage3/Graphics/Exceptions.o

Stage3/Graphics/Format.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Graphics/Format.c -o Stage3/Graphics/Format.o

Stage3/Graphics/Vbe.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Graphics/Vbe.c -o Stage3/Graphics/Vbe.o

Stage3/Int/Idt.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Int/Idt.c -o Stage3/Int/Idt.o

Stage3/Int/Irq.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Int/Irq.c -o Stage3/Int/Irq.o

Stage3/Int/Isr.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Stage3/Int/Isr.s -o Stage3/Int/Isr.o


# (Shared code, Legacy/Shared)

Shared/Disk/Disk.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Disk/Disk.c -o Shared/Disk/Disk.o

Shared/Graphics/Exceptions.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Graphics/Exceptions.c -o Shared/Graphics/Exceptions.o

Shared/Graphics/Format.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Graphics/Format.c -o Shared/Graphics/Format.o

Shared/Graphics/Graphics.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Graphics/Graphics.c -o Shared/Graphics/Graphics.o

Shared/Memory/Memory.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -mmmx -msse -msse2 -c Shared/Memory/Memory.c -o Shared/Memory/Memory.o

Shared/Rm/Rm.bin:
	@echo "Building $@"
	@$(AS) Shared/Rm/Rm.asm -f bin -o Shared/Rm/Rm.bin

Shared/Rm/RmWrapper.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Rm/RmWrapper.c -o Shared/Rm/RmWrapper.o

# Link the stages into .bin

Stage2/Stage2.bin: Stage2/Bootloader.o Shared/Disk/Disk.o Shared/Graphics/Exceptions.o Shared/Graphics/Format.o Shared/Graphics/Graphics.o Shared/Memory/Memory.o Shared/Rm/RmWrapper.o
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -Wl,--script=Stage2/Linker.ld -o Stage2/Stage2.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Stage2/Stage2.elf Stage2/Stage2.bin

Stage3/Stage3.bin: Stage3/Bootloader.o Stage3/Elf.o Stage3/Stub.o Stage3/Cpu/Cpu.o Stage3/Memory/A20/A20.o Stage3/Memory/A20/A20_Wrapper.o Stage3/Memory/Mmap/Mmap.o Stage3/Memory/Paging/Paging.o Stage3/Graphics/Vbe.o Stage3/Int/Idt.o Stage3/Int/Irq.o Stage3/Int/Isr.o Shared/Disk/Disk.o Shared/Graphics/Exceptions.o Shared/Graphics/Format.o Shared/Graphics/Graphics.o Shared/Memory/Memory.o Shared/Rm/RmWrapper.o
	@echo "Building $@"
	@$(CC) $(LDFLAGS) -Wl,--script=Stage3/Linker.ld -o Stage3/Stage3.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Stage3/Stage3.elf Stage3/Stage3.bin

Bootx32.bin: Stage3/Stage3.bin
	@cp Stage3/Stage3.bin Bootx32.bin
