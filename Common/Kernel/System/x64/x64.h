// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <

#ifndef SERRA_KERNEL_SYSTEM_X64_H
#define SERRA_KERNEL_SYSTEM_X64_H

  // Include standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"

  // Include structures and global variables from Cpu.c

  typedef struct _cpuidRegisterTable {

    uint64 Rax, Rbx, Rcx, Rdx;

  } __attribute__ ((packed)) cpuidRegisterTable;

  typedef struct _cpuFeaturesAvailable {

    // (SIMD- and register-specific features)

    bool Fxsave : 1; // Are `fxsave` instructions available?
    bool Sse : 1; // Are (base) SSE features available?
    bool Sse2 : 1; // Are SSE 2 features available?
    bool Sse3 : 1; // Are SSE 3 features available?
    bool Ssse3 : 1; // Are SSSE 3 features available?
    bool Sse4 : 1; // Are SSE4.1 features available?

    bool Xsave : 1; // Are `xsave` instructions and extended control registers available?
    bool Avx : 1; // Are (base) AVX features available?
    bool Avx2 : 1; // Are (base) AVX2 features available?
    bool Avx512f : 1; // Are (foundational) AVX512 features available?

    // ()

  } cpuFeaturesAvailable;

  extern cpuFeaturesAvailable CpuFeaturesAvailable_x64;

  // Include functions from Cpu.c

  cpuidRegisterTable QueryCpuid_x64(uint64 Rax, uint64 Rcx);

  void WriteToControlRegister_x64(uint8 Register, bool IsExtendedRegister, uint64 Value);
  uint64 ReadFromControlRegister_x64(uint8 Register, bool IsExtendedRegister);

  void WriteToMsr_x64(uint32 Msr, uint64 Value);
  uint64 ReadFromMsr_x64(uint32 Msr);

  void InitializeCpuFeatures_x64(void);

#endif
