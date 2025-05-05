# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# You'll need the following toolchains (with full C23 support):
# -> An i686-elf-gcc (or other i686 ELF) toolchain;
# -> An x86_64-elf-gcc (or other x64 ELF) toolchain;
# -> An x86_64-w64-mingw32-gcc (or other x64 PE) toolchain.

# Additionally, you'll also need mtools and nasm, as well as
# a 64-bit version of QEMU if you want to emulate it.

QEMU = qemu-system-x86_64
QFLAGS = -cpu qemu64 -m 128
OVMFDIR = /usr/share/OVMF/OVMF_CODE_4M.fd

CONFIG := makefile.config
include ${CONFIG}


# The .PHONY directive is used on targets that don't output anything. For example, running 'make all' builds our
# bootloader, but it doesn't output any specific files; it just goes through a lot of targets; the target that builds
# the final output isn't 'all', it's 'Boot.bin'. If Make sees that something is already there when executing a target,
# it skips it (for example, for the target 'example.o', if it sees example.o is already there, it skips compiling it),
# and this can cause problems for targets that don't output anything. These are called 'phony targets'.

.PHONY: All Compile Clean MakeUnpart MakeMbr MakeGpt Run RunBochs RunEfi RunGdb RunInt RunKvm all compile clean run runbochs runefi rungdb runint runkvm

# Names ->>
# (By the way, it's assumed that you're on Linux, or at least some sort of Unix-like system)

All: Clean Compile

Compile:

	@if [ $(BuildEfi) = true ]; then \
		echo "\n\033[0;1m""Compiling [Boot/Efi]""\033[0m\n"; \
		$(MAKE) -C Boot/Efi compile; \
	fi

	@if [ $(BuildBios) = true ]; then \
		echo "\n\033[0;1m""Compiling [Boot/Legacy]""\033[0m\n"; \
		$(MAKE) -C Boot/Legacy compile; \
	fi

	@echo "\n\033[0;1m""Compiling [Common]""\033[0m\n"
	@$(MAKE) -C Common compile

	@echo "\n\033[0;1m""Assembling [Serra.img]""\033[0m\n"
	@$(MAKE) Serra.img

Clean:

	@echo "\n\033[0;1m""Cleaning leftover files (*.bin, *.img, *.iso).." "\033[0m"
	rm -f *.bin
	rm -f *.img
	rm -f *.iso

	@if [ $(BuildEfi) = true ]; then \
	  echo "\n\033[0;1m""Cleaning [Boot/Efi]""\033[0m\n"; \
	  $(MAKE) -C Boot/Efi clean; \
	fi

	@if [ $(BuildBios) = true ]; then \
		echo "\n\033[0;1m""Cleaning [Boot/Legacy]""\033[0m\n"; \
		$(MAKE) -C Boot/Legacy clean; \
	fi

	@echo "\n\033[0;1m""Cleaning [Common]""\033[0m\n"
	@$(MAKE) -C Common clean


# Running with QEMU (or Bochs, with runbochs) -->

Run:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode).." "\033[0m"
	$(QEMU) $(QFLAGS) -drive file=Serra.img,format=raw

RunBochs:
	@echo "\n\033[0;1m""Launching Bochs (with automatic configuration file).." "\033[0m"
	@bochs -q

RunEfi:
	@echo "\n\033[0;1m""Launching QEMU (EFI mode).." "\033[0m"
	$(QEMU) $(QFLAGS) -drive if=pflash,format=raw,unit=0,file=$(OVMFDIR) -net none -drive file=Serra.img,format=raw

RunGdb:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with GDB.." "\033[0m"
	$(QEMU) $(QFLAGS) -s -S -drive file=Serra.img,format=raw

RunInt:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with -d int.." "\033[0m"
	@$(QEMU) $(QFLAGS) -drive file=Serra.img,format=raw -d int

RunKvm:
	@echo "\n\033[0;1m""Launching QEMU (legacy mode) with KVM.." "\033[0m"
	$(QEMU) $(QFLAGS) -drive file=Serra.img,format=raw -enable-kvm

# Lowercase names

all: All
clean: Clean
compile: Compile
run: Run
runbochs: RunBochs
runefi: RunEfi
rungdb: RunGdb
runint: RunInt
runkvm: RunKvm




# [Combine all of the different stages into one unified image]
# You can control the variables used here in `makefile.config`

export SFDISK_MBR_CONFIGURATION
export SFDISK_GPT_CONFIGURATION

Serra.img:

	@echo "Building $@"

# (Create a partition image)

	@dd if=/dev/zero of=Partition.img bs=1M count=$(PartitionSize) status=none

# (If we're building for a BIOS target, then add the bootsector (VBR), as
# well as the 2nd stage bootloader to the reserved sectors of the image.)

	@if [ $(BuildBios) = true ]; then \
		dd if=Boot/Legacy/Bootsector/Bootsector.bin of=Partition.img conv=notrunc bs=512 count=1 seek=0 status=none; \
		dd if=Boot/Legacy/Init/Init.bin of=Partition.img conv=notrunc bs=512 count=16 seek=16 status=none; \
		dd if=Boot/Legacy/Shared/Rm/Rm.bin of=Partition.img conv=notrunc bs=512 count=8 seek=32 status=none; \
	fi

# (Actually format the partition image itself, with the requested sector (-M)
# and cluster (-c) sizes, maintaining the boot sector (-k), adding the
# partition LBA (-H), and with *a minimum of 64 reserved sectors* (-R))

	@if [ $(ImageType) = unpart ]; then \
		mformat -i Partition.img -M $(SectorSize) -c $(ClusterSize) -k -R 64 ::; \
	else \
		mformat -i Partition.img -M 512 -c $(ClusterSize) -k -H $(PartitionLba) -R 64 ::; \
	fi

# (Next, we create a Boot/ folder, and if building for a BIOS target, add
# our 3rd stage bootloader (Bootx32.bin) to that folder.)

	@mmd -i Partition.img ::/Boot; \

	@if [ $(BuildBios) = true ]; then \
		mcopy -i Partition.img Boot/Legacy/Bootx32.bin ::/Boot/; \
	fi

# (If we're building for an EFI target, we also add the EFI bootloader to
# Boot/Efi/Bootx64.efi, which is the default boot location.)

	@if [ $(BuildEfi) = true ]; then \
		mmd -i Partition.img ::/Efi; \
		mmd -i Partition.img ::/Efi/Boot; \
		mcopy -i Partition.img Boot/Efi/Bootx64.efi ::/Efi/Boot/; \
	fi

# (Finally, we add the actual kernel files; this is necessary no matter
# what target we're building for.)

	@mmd -i Partition.img ::/Boot/Serra
	@mcopy -i Partition.img Common/Kernel/Kernel.elf ::/Boot/Serra/

# (Now, we can build the final image. Depending on the image type...)

# (unpart) The partition image is the final image, so we just rename it.

	@if [ $(ImageType) = unpart ]; then \
		mv Partition.img Serra.img; \
	fi

# (mbr) We make an MBR image (with the required partition tables), add
# our MBR code to it, and manually add the partition;

	@if [ $(ImageType) = mbr ]; then \
		dd if=/dev/zero of=Serra.img bs=1M count=$(ImageSize) status=none; \
		echo "$$SFDISK_MBR_CONFIGURATION" | sfdisk Serra.img; \
		dd if=Boot/Legacy/Bootsector/Mbr.bin of=Serra.img conv=notrunc bs=1 count=440 status=none; \
		dd if=Boot/Legacy/Bootsector/Mbr.bin of=Serra.img conv=notrunc bs=1 count=2 skip=510 seek=510 status=none; \
		dd if=Partition.img of=Serra.img conv=notrunc bs=512 seek=$(PartitionLba) status=none; \
		rm Partition.img; \
	fi

# (gpt) We make a GPT image (with the required partition tables), and
# manually add the partition.

# (Please keep in mind this has no BIOS support)

	@if [ $(ImageType) = gpt ]; then \
		dd if=/dev/zero of=Serra.img bs=1M count=$(ImageSize) status=none; \
		echo "$$SFDISK_GPT_CONFIGURATION" | sfdisk Serra.img; \
		dd if=Partition.img of=Serra.img conv=notrunc bs=512 seek=$(PartitionLba) status=none; \
		rm Partition.img; \
	fi
