// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"

#include "Cpu.h"

// ...

cpuidData CallCpuid(uint32 Eax) {

  cpuidData Data;
  __asm__("cpuid" : "=b"(Data.Ebx), "=c"(Data.Ecx), "=d"(Data.Edx) : "a"(Eax));

  return Data;

}

// ...

char* CpuidGetVendor(char* Buffer, cpuidData Data) {

  Memcpy(&Buffer[0], (void*)&Data.Ebx, 4);
  Memcpy(&Buffer[4], (void*)&Data.Edx, 4);
  Memcpy(&Buffer[8], (void*)&Data.Ecx, 4);

  Buffer[12] = '\0';
  return Buffer;

}
