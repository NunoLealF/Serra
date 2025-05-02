// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "Cpu/Cpu.h"
  #include "Efi/Efi.h"
  #include "Graphics/Graphics.h"
  #include "../../Common/InfoTable.h"

  // Declare functions and global variables in Bootloader.c

  efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable);

  kernelEfiInfoTable EfiInfoTable;
  kernelInfoTable KernelInfoTable;

  bool Debug = true;
  bool SupportsConIn = true;
  bool SupportsConOut = true;

#endif
