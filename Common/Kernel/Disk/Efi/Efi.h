// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_EFI_H
#define SERRA_KERNEL_DISK_EFI_H

  // Import standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"

  // Include functions from Efi.c

  #include "../../../Common.h"
  [[nodiscard]] bool InitializeDiskSubsystem_Efi(void);
  bool TerminateDiskSubsystem_Efi(void);

  [[nodiscard]] bool ReadSectors_Efi(void* Buffer, uint64 Lba, uint64 NumSectors, uint16 BlockIoIndex, uint32 MediaId);

#endif
