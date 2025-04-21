# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need an i686-elf-gcc (32-bit) and x86_64-w64-mingw32-gcc (64-bit)
# toolchain, along with mtools and nasm, as well as qemu if you want to emulate
# it.

QEMU = qemu-system-x86_64
QFLAGS = -cpu qemu64 -m 128

# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Compile Clean Run RunBochs RunGdb RunInt RunKvm all compile clean run rungdb runint runkvm

# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile:

	@echo "\n\033[0;1m""Compiling [Boot/Legacy]""\033[0m\n"
	@$(MAKE) -C Boot/Legacy compile

	@echo "\n\033[0;1m""Compiling [Common]""\033[0m\n"
	@$(MAKE) -C Common compile

	@echo "\n\033[0;1m""Assembling [Legacy.img]""\033[0m\n"
	@$(MAKE) Legacy.img

Clean:

	@echo "\n\033[0;1m""Cleaning leftover files (*.bin, *.img, *.iso).." "\033[0m"
	rm -f *.bin
	rm -f *.img
	rm -f *.iso

	@echo "\n\033[0;1m""Cleaning [Boot/Legacy]""\033[0m\n"
	@$(MAKE) -C Boot/Legacy clean

	@echo "\n\033[0;1m""Cleaning [Common]""\033[0m\n"
	@$(MAKE) -C Common clean


# Running with QEMU (or Bochs, with runbochs) -->

Run:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode).." "\033[0m"
	$(QEMU) $(QFLAGS) -drive file=Legacy.img,format=raw

RunBochs:
	@echo "\n\033[0;1m""Launching Bochs (with automatic configuration file).." "\033[0m"
	@bochs -q

RunGdb:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with GDB.." "\033[0m"
	$(QEMU) $(QFLAGS) -s -S -drive file=Legacy.img,format=raw

RunInt:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with -d int.." "\033[0m"
	@$(QEMU) $(QFLAGS) -drive file=Legacy.img,format=raw -d int

RunKvm:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with KVM.." "\033[0m"
	$(QEMU) $(QFLAGS) -drive file=Legacy.img,format=raw -enable-kvm

# Lowercase names

all: All
clean: Clean
compile: Compile
run: Run
runbochs: RunBochs
rungdb: RunGdb
runint: RunInt
runkvm: RunKvm



# [...]

# Mix all of the different stages together (73728 sectors, so, a 36 MiB img)

Legacy.img:

	@echo "Building $@"
	@dd if=/dev/zero of=Legacy.img bs=512 count=73728 status=none

	@dd if=Boot/Legacy/Bootsector/Bootsector.bin of=Legacy.img conv=notrunc bs=512 count=1 seek=0 status=none
	@dd if=Boot/Legacy/Init/Init.bin of=Legacy.img conv=notrunc bs=512 count=16 seek=16 status=none
	@dd if=Boot/Legacy/Shared/Rm/Rm.bin of=Legacy.img conv=notrunc bs=512 count=8 seek=32 status=none

# (This formats it as a valid FAT16 filesystem while keeping the non-BPB part of the bootsector;
# that being said, *it's a temporary solution*, since we'll eventually need to merge the
# legacy and EFI code).

# Specifically, this sets a cluster size of 16 sectors, keeps the current bootsector (except
# for the BPB, and sets the number of reserved sectors to 64.

	@mformat -i Legacy.img -c 16 -k -R 64 ::

# (Add the 3rd stage bootloader; it's assumed that the actual bootloader will be in
# Boot/Legacy/Boot/Bootx32.bin, with the kernel/common stage coming later on.)

	@mmd -i Legacy.img ::/Boot
	@mcopy -i Legacy.img Boot/Legacy/Bootx32.bin ::/Boot/

# (Add the actual kernel files)

	@mmd -i Legacy.img ::/Boot/Serra
	@mcopy -i Legacy.img Common/Kernel/Kernel.elf ::/Boot/Serra/
