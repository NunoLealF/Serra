// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "Cpu/Cpu.h"
  #include "Elf.h"
  #include "Efi/Efi.h"
  #include "Graphics/Graphics.h"
  #include "Memory/Memory.h"
  #include "../../Common/InfoTable.h"

  // Declare functions in Bootloader.c and Stub.asm.

  efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable);
  uint64 efiAbi TransitionStub(commonInfoTable* InfoTable, void* KernelEntrypoint, void* KernelStackTop);

  // Declare global variables, used throughout the bootloader.

  efiSystemTable* gST = NULL;
  efiBootServices* gBS = NULL;
  efiRuntimeServices* gRT = NULL;

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

  bool SupportsConIn = true;
  bool SupportsConOut = true;

  // Kernel-related definitions.

  #define MinFirmwareMemory (64ULL * 1048576) // A size of 64 MiB

  #define KernelLocation u"\\BOOT\\SERRA\\KERNEL.ELF" // Kernel file location
  #define KernelStackSize 0x100000 // Must be a multiple of 4 KiB

  commonInfoTable CommonInfoTable = {0};

  void* Kernel = NULL;
  void* KernelArea = NULL;
  void* KernelEntrypoint = NULL;
  void* KernelStack = NULL;

#endif
