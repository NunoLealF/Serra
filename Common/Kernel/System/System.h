// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_SYSTEM_H
#define SERRA_KERNEL_SYSTEM_H

  // Include the relevant definitions for each architecture.

  #if defined(__amd64__) || defined(__x86_64__)

    // Include definitions and structures from Amd64.c

    #define SystemPageSize 4096

    typedef struct _cpuFeaturesAvailable {

      // (SIMD- and register-specific features)

      bool Fxsave : 1; // Are `fxsave` instructions available?
      bool Sse : 1; // Are (base) SSE features available?
      bool Sse2 : 1; // Are SSE 2 features available?
      bool Sse3 : 1; // Are SSE 3 features available?
      bool Ssse3 : 1; // Are SSSE 3 features available?
      bool Sse4 : 1; // Are SSE4.1/4.2 (not SSE4a!) features available?

      bool Xsave : 1; // Are `xsave` instructions and extended control registers available?
      bool Erms : 1; // Is Enhanced REP MOVS*/STOS* (ERMS) being used?
      bool Avx : 1; // Are (base) AVX features available?
      bool Avx2 : 1; // Are (base) AVX2 features available?
      bool Avx512f : 1; // Are (foundational) AVX512 features available?

    } cpuFeaturesAvailable;

    typedef struct _cpuidRegisterTable {
      uint64 Rax, Rbx, Rcx, Rdx;
    } __attribute__ ((packed)) cpuidRegisterTable;

    // Include functions and global variables from Amd64.c

    extern cpuFeaturesAvailable CpuFeaturesAvailable;
    void InitializeCpuFeatures(void);

    void WriteToControlRegister(uint8 Register, bool IsExtendedRegister, uint64 Value);
    uint64 ReadFromControlRegister(uint8 Register, bool IsExtendedRegister);

    void WriteToMsr(uint32 Msr, uint64 Value);
    uint64 ReadFromMsr(uint32 Msr);

    cpuidRegisterTable QueryCpuid(uint64 Rax, uint64 Rcx);

  #elif defined(__aarch64__)

    #define SystemPageSize 16384 // (I've heard M1 Macs do this)
    #error "ARM64 targets are not supported :("

  #elif defined(__riscv) || defined(__riscv__)

    #error "RISC-V targets are not supported :("

  #else

    #error "Could not detect the system architecture."

  #endif

#endif
