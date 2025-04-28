// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "Efi/Efi.h"

  #include "../../Common/InfoTable.h"

  // Declare functions and global variables in Bootloader.c

  efiStatus (efiAbi SEfiBootloader) ([[maybe_unused]] efiHandle ImageHandle, efiSystemTable* SystemTable);

  kernelInfoTable KernelInfoTable;
  kernelEfiInfoTable EfiInfoTable;

  [[maybe_unused]] bool SupportsConIn = true;
  [[maybe_unused]] bool SupportsConOut = true;

  [[maybe_unused]] bool Debug = true; // TODO: Make a mechanism to control this, as in the bios bootloader

#endif
