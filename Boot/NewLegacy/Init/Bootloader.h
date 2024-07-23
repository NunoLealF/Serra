// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers (...)

  #include "../Shared/InfoTables.h"
  #include "../Shared/Rm/Rm.h"
  #include "Disk/Disk.h"
  #include "Memory/Memory.h"
  #include "Graphics/Graphics.h"

  // Declare functions in Bootloader.c

  void __attribute__((noreturn)) Init(void);
  void __attribute__((noreturn)) Bootloader(void);

  // todo...

#endif
