// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_CORE_H
#define SERRA_KERNEL_CORE_H

  // Import standard and bootloader-related headers.

  #include "Libraries/Graphics/Graphics.h"
  #include "Libraries/Stdint.h"
  #include "Libraries/String.h"
  #include "../Common.h"

  // Import kernel constructors.

  #include "Constructors/Firmware/Firmware.h"

  // Import firmware-specific headers.

  #include "Firmware/Bios/Bios.h"
  #include "Firmware/Efi/Efi.h"

  // Import system-specific headers.

  #include "System/System.h"

  // Declare functions in Core.c.

  void KernelCore(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

#endif
