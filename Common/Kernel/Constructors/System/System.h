// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_CONSTRUCTORS_SYSTEM_H
#define SERRA_KERNEL_CONSTRUCTORS_SYSTEM_H

  // Include standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"
  #include "../../System/System.h"

  // Include functions and global variables from x64.c

  #if defined(__amd64__) || defined(__x86_64__)

    typedef struct _cpuFeaturesAvailable {

      // (SIMD- and register-specific features)

      bool Fxsave : 1; // Are `fxsave` instructions available?
      bool Sse : 1; // Are (base) SSE features available?
      bool Sse2 : 1; // Are SSE 2 features available?
      bool Sse3 : 1; // Are SSE 3 features available?
      bool Ssse3 : 1; // Are SSSE 3 features available?
      bool Sse4 : 1; // Are SSE4a/4.1/4.2 features available?

      bool Xsave : 1; // Are `xsave` instructions and extended control registers available?
      bool Avx : 1; // Are (base) AVX features available?
      bool Avx2 : 1; // Are (base) AVX2 features available?
      bool Avx512f : 1; // Are (foundational) AVX512 features available?

    } cpuFeaturesAvailable;

    extern cpuFeaturesAvailable CpuFeaturesAvailable;

    void InitializeCpuFeatures(void);

  #endif

#endif
