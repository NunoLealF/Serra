// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "Cpu/Cpu.h"
  #include "Efi/Efi.h"
  #include "Graphics/Graphics.h"
  #include "Memory/Memory.h"
  #include "../../Common/InfoTable.h"

  // Declare functions and global variables in Bootloader.c

  efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable);

  kernelEfiInfoTable EfiInfoTable;
  kernelInfoTable KernelInfoTable;

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

  bool SupportsConIn = true;
  bool SupportsConOut = true;

#endif
