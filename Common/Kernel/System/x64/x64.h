// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#if !defined(__amd64__) || !defined(__x86_64__)
  #error "This code must be compiled with an x64 cross compiler."
#endif

#ifndef SERRA_KERNEL_SYSTEM_X64_H
#define SERRA_KERNEL_SYSTEM_X64_H

  // Include standard and/or necessary headers.

  #include "../../Stdint.h"

  // Include structures from Cpu.c

  typedef struct _registerTable {

    uint64 Rax, Rbx, Rcx, Rdx;

  } __attribute__ ((packed)) cpuidRegisterTable;

  // Include functions from Cpu.c

  cpuidRegisterTable QueryCpuid(uint64 Rax, uint64 Rcx);

  void WriteToControlRegister(uint8 Register, bool IsExtendedRegister, uint64 Value);
  uint64 ReadFromControlRegister(uint8 Register, bool IsExtendedRegister);

#endif
