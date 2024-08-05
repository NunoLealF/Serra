// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_CPU_H
#define SERRA_CPU_H

  // structures from Cpu.c

  typedef struct {

    uint32 Eax;
    uint32 Ebx;
    uint32 Ecx;
    uint32 Edx;

  } __attribute__ ((packed)) registerTable;

  // functions from Cpu.c

  uint32 ReadEflags(void);
  void ChangeEflags(uint8 Bit, bool Set);

  bool SupportsCpuid(void);
  registerTable GetCpuid(uint32 Eax, uint32 Ecx);


#endif
