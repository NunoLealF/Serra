// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers

  #include "Graphics/Graphics.h"
  #include "Memory/Memory.h"
  #include "Int/Int.h"
  #include "Rm/Rm.h"
  
  #include "Exceptions.h"

  // Declare functions in Bootloader.c

  void __attribute__((noreturn)) Init(void);
  void __attribute__((noreturn)) Bootloader(void);

  // todo...

#endif
