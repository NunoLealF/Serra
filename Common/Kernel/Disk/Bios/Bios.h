// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_BIOS_H
#define SERRA_KERNEL_DISK_BIOS_H

  // Import standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"

  // Include functions and global variables from Bios.c

  constexpr uint32 Int13Wrapper_Data = 0x70000;
  constexpr uint32 Int13Wrapper_Location = 0x7FE00;

  [[nodiscard]] bool Setup_Int13Wrapper(void);
  [[nodiscard]] bool Read_Int13Wrapper(void* Pointer, uint64 Lba, uint64 Size);

#endif
