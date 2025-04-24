// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers..

  #include "../Shared/InfoTable.h"
  #include "../Shared/Rm/Rm.h"
  #include "../Shared/Disk/Disk.h"

  #include "Cpu/Cpu.h"
  #include "Memory/Memory.h"
  #include "Graphics/Graphics.h"
  #include "Int/Int.h"
  #include "Elf.h"

  // Declare functions in Bootloader.c and Stub.asm

  void Bootloader(void);
  void LongmodeStub(uintptr InfoTable, uintptr Pml4);

  // Kernel-related definitions.

  #include "../../../Common/InfoTable.h"

  #define KernelStackSize 0x100000 // (Must be a multiple of 4KiB)
  uint64 KernelEntrypoint;
  uint64 KernelStack;

#endif
