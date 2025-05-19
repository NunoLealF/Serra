// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_H
#define SERRA_KERNEL_H

  // Import other headers...

  #include "../InfoTable.h"
  #include "../../Boot/Efi/Efi/Efi.h"

  // Declare functions in Entry.c (TODO: and Kernel.c)

  entrypointReturnValue Entrypoint(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  efiSystemTable* gST = NULL;
  efiBootServices* gBS = NULL;
  efiRuntimeServices* gRT = NULL;

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

#endif
