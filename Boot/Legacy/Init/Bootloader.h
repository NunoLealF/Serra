// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "../Shared/Memory/Memory.h"
  #include "../Shared/Rm/Rm.h"
  
  #include "../Shared/Disk/Disk.h"
  #include "../Shared/InfoTables.h"

  #include "Graphics/Graphics.h"

  // Declare functions in Bootloader.c

  void __attribute__((noreturn)) Init(void);
  void __attribute__((noreturn)) Bootloader(void);

#endif
