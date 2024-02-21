// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_CPU_H
#define SERRA_CPU_H

  // Data structures in Cpu.c

  typedef struct {

    uint32 Eax;
    uint32 Ebx;
    uint32 Ecx;
    uint32 Edx;

  } cpuidData;

  // Functions in Cpu.c

  cpuidData CallCpuid(uint32 Eax);
  char* CpuidGetVendor(char* Buffer, cpuidData Data);

  // Functions in Cpuid.s

  uint32 CheckCpuid(void);

#endif
