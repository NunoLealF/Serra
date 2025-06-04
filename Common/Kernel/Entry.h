// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_ENTRYPOINT_H
#define SERRA_KERNEL_ENTRYPOINT_H

  // Import standard and bootloader-related headers.

  #include "Libraries/Stdint.h"
  #include "Libraries/String.h"
  #include "Libraries/Stdio.h"
  #include "../Common.h"

  // Import firmware-specific headers.

  #include "Firmware/Firmware.h"

  // Import graphics-specific headers.

  #include "Graphics/Graphics.h"
  #include "Graphics/Console/Console.h"
  #include "Graphics/Fonts/Fonts.h"

  // Import memory-specific headers.

  #include "Memory/Memory.h"

  // Import system-specific headers.

  #include "System/System.h"

  // Declare functions in Entry.c and Core.c.

  entrypointReturnStatus Entrypoint(commonInfoTable* InfoTable);
  void KernelCore(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  #ifdef KernelMb
    #define KernelMemoryLimit (1048576ULL * KernelMb) // Defined by the preprocessor, use -DKernelMb=(%d).
  #else
    #define KernelMemoryLimit (1048576ULL * 0) // If `KernelMb` isn't defined, assume no limit.
  #endif

#endif
