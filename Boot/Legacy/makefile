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

# -Os: Optimize the resulting file/binary for space. [Warning, do *not* use O3]

# -Wall: Enable all warnings.

# -Wextra: Enable even more warnings. (Not sure why wall doesn't take care of this)

# -Wpedantic: Be pedantic about the C standard / any possible mistakes

# -funsigned-char: Use unsigned chars for characters.

LDFLAGS = -Wl,--no-warn-rwx-segments
CFLAGS = -std=c11 -ffunction-sections -funsigned-char -ffreestanding -Os -Wall -Wextra -Wpedantic -Wno-discarded-qualifiers


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Clean Dump Run RunGdb RunInt RunKvm all dump clean run rungdb runint runkvm


# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Bootsector/Bootsector.bin Init/Init.bin Shared/Rm/Rm.bin Core/Serra.bin Legacy.bin

Clean:
	@echo "Cleaning leftover files (*.o, *.elf, *.bin).."

	@-rm -f Bootsector/*.o
	@-rm -f Bootsector/*.bin

	@-rm -f Init/*.o
	@-rm -f Init/*.elf
	@-rm -f Init/*.bin

	@-rm -f Init/Disk/*.o
	@-rm -f Init/Memory/*.o
	@-rm -f Init/Graphics/*.o

	@-rm -f Core/*.o
	@-rm -f Core/*.elf
	@-rm -f Core/*.bin

	@-rm -f Core/Cpu/*.o
	@-rm -f Core/Graphics/*.o
	@-rm -f Core/Int/*.o
	@-rm -f Core/Memory/*.o
	@-rm -f Core/Memory/A20/*.o
	@-rm -f Core/Memory/Mmap/*.o

	@-rm -f Shared/Rm/*.o
	@-rm -f Shared/Rm/*.elf
	@-rm -f Shared/Rm/*.bin

	@-rm -f *.bin


# (...)

Dump:
	$(OBJD) -S Core/Serra.elf

Dumpbin:
	$(OBJD) -Mintel,i686 -b binary -m i386 --adjust-vma=0x7E00 -D Legacy.bin

Run:
	@echo "Launching Qemu.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Legacy.bin,format=raw

RunGdb:
	@echo "Launching Qemu.."
	@qemu-system-i386 -s -S -cpu pentium3 -m 128 -drive file=Legacy.bin,format=raw

RunInt:
	@echo "Launching Qemu with -d int.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Legacy.bin,format=raw -d int

RunKvm:
	@echo "Launching Qemu with KVM.."
	@qemu-system-i386 -cpu pentium3 -m 128 -drive file=Legacy.bin,format=raw -enable-kvm

# Lowercase names

all: All
clean: Clean
dump: Dump
dumpbin: Dumpbin
run: Run
rungdb: RunGdb
runint: RunInt
runkvm: RunKvm



# [The files themselves]

# (First-stage bootloader, Legacy/Bootsector)

Bootsector/Bootsector.bin:
	@echo "Building $@"
	@$(AS) Bootsector/Bootsector.asm -f bin -o Bootsector/Bootsector.bin


# (Second-stage bootloader, Legacy/Init)

Init/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Bootloader.c -o Init/Bootloader.o

Init/Disk/Disk.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Disk/Disk.c -o Init/Disk/Disk.o

Init/Memory/Memory.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Memory/Memory.c -o Init/Memory/Memory.o

Init/Graphics/Exceptions.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Graphics/Exceptions.c -o Init/Graphics/Exceptions.o

Init/Graphics/Format.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Graphics/Format.c -o Init/Graphics/Format.o

Init/Graphics/Graphics.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Init/Graphics/Graphics.c -o Init/Graphics/Graphics.o


# (Third-stage bootloader, Legacy/Core)

Core/Bootloader.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Bootloader.c -o Core/Bootloader.o

Core/Cpu/Cpu.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Cpu/Cpu.c -o Core/Cpu/Cpu.o

Core/Memory/Memory.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Memory/Memory.c -o Core/Memory/Memory.o

Core/Memory/A20/A20.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Memory/A20/A20.s -o Core/Memory/A20/A20.o

Core/Memory/A20/A20_Wrapper.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Memory/A20/A20_Wrapper.c -o Core/Memory/A20/A20_Wrapper.o

Core/Memory/Mmap/Mmap.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Memory/Mmap/Mmap.c -o Core/Memory/Mmap/Mmap.o

Core/Graphics/Exceptions.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Graphics/Exceptions.c -o Core/Graphics/Exceptions.o

Core/Graphics/Format.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Graphics/Format.c -o Core/Graphics/Format.o

Core/Graphics/Graphics.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Graphics/Graphics.c -o Core/Graphics/Graphics.o

Core/Int/Idt.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Int/Idt.c -o Core/Int/Idt.o

Core/Int/Irq.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Int/Irq.c -o Core/Int/Irq.o

Core/Int/Isr.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Core/Int/Isr.s -o Core/Int/Isr.o


# (Shared code, Legacy/Shared)

Shared/Rm/Rm.bin:
	@echo "Building $@"
	@$(AS) Shared/Rm/Rm.asm -f bin -o Shared/Rm/Rm.bin

Shared/Rm/RmWrapper.o:
	@echo "Building $@"
	@$(CC) $(CFLAGS) -c Shared/Rm/RmWrapper.c -o Shared/Rm/RmWrapper.o


# Link the stages into .bin

Init/Init.bin: Init/Bootloader.o Init/Disk/Disk.o Init/Memory/Memory.o Init/Graphics/Exceptions.o Init/Graphics/Format.o Init/Graphics/Graphics.o Shared/Rm/RmWrapper.o
	@echo "Building $@"
	@$(CC) -T Init/Linker.ld $(LDFLAGS) -o Init/Init.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Init/Init.elf Init/Init.bin

Core/Serra.bin: Core/Bootloader.o Core/Cpu/Cpu.o Core/Memory/Memory.o Core/Memory/A20/A20.o Core/Memory/A20/A20_Wrapper.o Core/Memory/Mmap/Mmap.o Core/Graphics/Exceptions.o Core/Graphics/Format.o Core/Graphics/Graphics.o Core/Int/Idt.o Core/Int/Irq.o Core/Int/Isr.o Shared/Rm/RmWrapper.o
	@echo "Building $@"
	@$(CC) -T Core/Linker.ld $(LDFLAGS) -o Core/Serra.elf -ffreestanding -nolibc -nostdlib -lgcc $^
	@$(OBJC) -O binary Core/Serra.elf Core/Serra.bin


# Mix all of the different stages together (8192 sectors, so, a 4MiB img)

Legacy.bin: Bootsector/Bootsector.bin Init/Init.bin Shared/Rm/Rm.bin Core/Serra.bin

	@echo "Building $@"
	@dd if=/dev/zero of=Legacy.bin bs=512 count=8192 status=none

	@dd if=Bootsector/Bootsector.bin of=Legacy.bin conv=notrunc bs=512 count=1 seek=0 status=none
	@dd if=Init/Init.bin of=Legacy.bin conv=notrunc bs=512 count=16 seek=8 status=none
	@dd if=Shared/Rm/Rm.bin of=Legacy.bin conv=notrunc bs=512 count=8 seek=24 status=none

# (This formats it as a valid FAT16 filesystem while keeping the non-BPB part of the bootsector;
# that being said, *it's a temporary solution*, since we'll eventually need to merge the
# legacy and EFI code).

# Specifically, this sets a cluster size of 1 sector, keeps the current bootsector except for
# the BPB, and sets the number of reserved sectors to 32.

	@mformat -i Legacy.bin -c 1 -k -R 32 ::

# (Add the 3rd stage bootloader)

	@mmd -i Legacy.bin ::/Boot
	@mcopy -i Legacy.bin Core/Serra.bin ::/Boot
