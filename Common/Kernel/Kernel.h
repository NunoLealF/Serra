// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_H
#define SERRA_KERNEL_H

  // Import standard and bootloader-related headers.

  #include "Stdint.h"
  #include "../Common.h"

  // Import kernel constructors.

  #include "Constructors/Firmware/Firmware.h"

  // Import firmware-specific headers.

  #include "Firmware/Bios/Bios.h"
  #include "Firmware/Efi/Efi.h"

  // Declare functions in Kernel.c.

  void Kernel(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

#endif
