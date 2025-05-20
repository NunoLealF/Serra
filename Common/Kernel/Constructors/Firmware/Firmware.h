// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_CONSTRUCTORS_FIRMWARE_H
#define SERRA_KERNEL_CONSTRUCTORS_FIRMWARE_H

  // Include standard and/or necessary headers.

  #include "../../Stdint.h"
  #include "../../Firmware/Bios/Bios.h"
  #include "../../Firmware/Efi/Efi.h"

  // Include global variables defined in Bios.c (TODO)

  // Include global variables defined in Efi.c

  extern efiSystemTable* gST;
  extern efiBootServices* gBS;
  extern efiRuntimeServices* gRT;

  // Include functions from Bios.c and Efi.c

  uint64 InitializeEfiTables(efiSystemTable* SystemTable);

#endif
