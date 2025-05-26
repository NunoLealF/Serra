// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_ENTRYPOINT_H
#define SERRA_KERNEL_ENTRYPOINT_H

  // Import standard and bootloader-related headers.

  #include "Libraries/Graphics/Graphics.h"
  #include "Libraries/Stdint.h"
  #include "Libraries/String.h"
  #include "../Common.h"

  // Import kernel constructors.

  #include "Constructors/Firmware/Firmware.h"
  #include "Constructors/Graphics/Graphics.h"
  #include "Constructors/System/System.h"

  // Import firmware-specific headers.

  #include "Firmware/Bios/Bios.h"
  #include "Firmware/Efi/Efi.h"

  // Import system-specific headers.

  #include "System/System.h"

  // Declare functions in Entry.c and Core.c.

  entrypointReturnStatus Entrypoint(commonInfoTable* InfoTable);
  void KernelCore(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  extern bool DebugFlag;

  #ifdef KernelMb
    #define KernelMemoryLimit (1048576ULL * KernelMb) // Defined by the preprocessor, use -DKernelMb=(%d).
  #else
    #define KernelMemoryLimit (1048576ULL * 0) // If `KernelMb` isn't defined, assume no limit.
  #endif

#endif
