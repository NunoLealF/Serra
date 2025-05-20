// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers..

  #include "../Shared/Rm/Rm.h"
  #include "../Shared/Disk/Disk.h"

  #include "Cpu/Cpu.h"
  #include "Memory/Memory.h"
  #include "Graphics/Graphics.h"
  #include "Int/Int.h"
  #include "Elf.h"

  #include "../Shared/InfoTable.h"
  #include "../../../Common/InfoTable.h"

  // Declare functions in Bootloader.c and Stub.asm

  void S3Bootloader(void);
  entrypointReturnStatus TransitionStub(commonInfoTable* InfoTable, void* Pml4);

  // Global variables, for use throughout the entire bootloader

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

  #ifdef Graphical
    bool GraphicalFlag = Graphical; // Defined by the preprocessor, use -DGraphical=true or false.
  #else
    bool GraphicalFlag = true; // If 'Graphical' isn't defined, then assume it's true
  #endif

  terminalDataStruct TerminalTable = {0};

  // Kernel-related definitions.

  #define KernelStackSize 0x100000 // Must be a multiple of 4 KiB

  commonInfoTable CommonInfoTable = {0};
  uint64 KernelEntrypoint = 0;
  uint64 KernelStack = 0;

#endif
