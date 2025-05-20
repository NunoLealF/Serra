// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_FIRMWARE_EFI_H
#define SERRA_KERNEL_FIRMWARE_EFI_H

  // Import standard headers.

  #include "../../Stdint.h"

  // Import EFI headers from ../../../../Boot/Efi/Efi/
  // (These are from the EFI bootloader stage)

  #include "../../../../Boot/Efi/Efi/Efi.h"
  #include "../../../../Boot/Efi/Efi/Protocols.h"
  #include "../../../../Boot/Efi/Efi/Tables.h"

  // Include EFI-related definitions.

  typedef efiMemoryDescriptor efiMmapEntry;
  #define efiMmapEntrySize (sizeof(efiMmapEntry))

  // Include kernel-specific definitions, and functions from Efi.c (TODO)

#endif
