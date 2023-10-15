// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../../Rm/Rm.h"

// (Todo: memory map functions)
// We just use e820

// .

uint32 GetMmapEntry(void* Buffer, uint32 Size, uint32 Continuation) {

  // ...

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = 0xE820;
  Table->Ebx = Continuation;
  Table->Ecx = Size;
  Table->Edx = 0x534D4150;

  Table->Di = (uint16)Buffer;
  Table->Int = 0x15;

  // ...

  RealMode();

  // ...

  if (hasFlag(Table->Eflags, carryFlag)) {

    return 0;

  }

  return Table->Ebx;

}
