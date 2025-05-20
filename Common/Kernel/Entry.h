// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_ENTRYPOINT_H
#define SERRA_KERNEL_ENTRYPOINT_H

  // Import other headers...

  #include "../InfoTable.h"
  #include "../Status.h"
  #include "../../Boot/Efi/Efi/Efi.h"

  // Declare functions in Entry.c and Kernel.c.

  entrypointReturnStatus Entrypoint(commonInfoTable* InfoTable);
  void Kernel(commonInfoTable* InfoTable);

  // Declare global variables, used throughout the kernel.

  extern efiSystemTable* gST;
  extern efiBootServices* gBS;
  extern efiRuntimeServices* gRT;

  extern bool DebugFlag;

  #ifdef KernelMb
    #define KernelMemoryLimit (1048576ULL * KernelMb) // Defined by the preprocessor, use -DKernelMb=(%d).
  #else
    #define KernelMemoryLimit (1048576ULL * 0) // If `KernelMb` isn't defined, assume no limit.
  #endif

#endif
