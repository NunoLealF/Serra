// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers..

  #include "../Shared/InfoTables.h"
  #include "../Shared/Rm/Rm.h"
  #include "../Shared/Disk/Disk.h"

  #include "Cpu/Cpu.h"
  #include "Memory/Memory.h"
  #include "Graphics/Graphics.h"
  #include "Int/Int.h"

  // Declare functions in Bootloader.c

  void Bootloader(void);

#endif
