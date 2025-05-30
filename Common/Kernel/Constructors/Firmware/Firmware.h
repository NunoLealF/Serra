// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_CONSTRUCTORS_FIRMWARE_H
#define SERRA_KERNEL_CONSTRUCTORS_FIRMWARE_H

  // Include standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"
  #include "../../../../Boot/Efi/Efi/Efi.h"
  #include "../../../../Boot/Efi/Efi/Protocols.h"
  #include "../../../../Boot/Efi/Efi/Tables.h"

  // Include functions and global variables from Efi.c

  extern efiSystemTable* gST;
  extern efiBootServices* gBS;
  extern efiRuntimeServices* gRT;

  uint64 InitializeEfiTables(efiSystemTable* SystemTable);

#endif
