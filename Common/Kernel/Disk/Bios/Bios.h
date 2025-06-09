// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_BIOS_H
#define SERRA_KERNEL_DISK_BIOS_H

  // Import standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"

  // Include functions and global variables from Bios.c

  constexpr uint32 Int13Wrapper_Data = 0x70000;
  constexpr uint16 Int13Wrapper_Location = 0x1000;

  [[nodiscard]] bool InitializeDiskSubsystem_Bios(void);
  [[nodiscard]] bool ReadDisk_Bios(void* Buffer, uint64 Lba, uint64 NumSectors, uint8 DriveNumber);

#endif
