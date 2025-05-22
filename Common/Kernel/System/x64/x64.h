// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <

#ifndef SERRA_KERNEL_SYSTEM_X64_H
#define SERRA_KERNEL_SYSTEM_X64_H

  // Include standard and/or necessary headers.

  #include "../../Libraries/Stdint.h"
  #include "../../Constructors/System/System.h"

  // Include structures and global variables from Cpu.c

  typedef struct _x64_cpuidRegisterTable {
    uint64 Rax, Rbx, Rcx, Rdx;
  } __attribute__ ((packed)) x64_cpuidRegisterTable;

  // Include functions from Cpu.c

  x64_cpuidRegisterTable x64_QueryCpuid(uint64 Rax, uint64 Rcx);

  void x64_WriteToControlRegister(uint8 Register, bool IsExtendedRegister, uint64 Value);
  uint64 x64_ReadFromControlRegister(uint8 Register, bool IsExtendedRegister);

  void x64_WriteToMsr(uint32 Msr, uint64 Value);
  uint64 x64_ReadFromMsr(uint32 Msr);

#endif
