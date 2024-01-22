// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Cpu.h"

// ...

cpuidData CallCpuid(uint32 Eax) {

  cpuidData Data;
  __asm__("cpuid" : "=b"(Data.Ebx), "=c"(Data.Ecx), "=d"(Data.Edx) : "a"(Eax));

  return Data;

}
