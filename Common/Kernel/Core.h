// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_CORE_H
#define SERRA_KERNEL_CORE_H

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

  // Declare functions in Core.c.

  void KernelCore(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

#endif
