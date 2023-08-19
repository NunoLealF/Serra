// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../../Rm/Rm.h"

// (Todo: memory map functions)

// .

uint32 getMmap_E820(void* Buffer, uint32 NumEntries) {

  // First call

  realModeTable* Table = initializeRealModeTable();

  Table->Eax = 0xE820;
  Table->Ebx = 0;
  Table->Ecx = 24;
  Table->Edx = 0x534D4150;

  Table->Di = (uint16)Buffer;

  Table->Int = 0x15;

  realMode();

  // Check to see if it worked, either with the carry flag

  if (Table->Eax != 0x534D4150) {

    // It didn't work on the first try; likely broken.

    return 0;

  }

  // If we're here, it's because we did; let's fill the rest of this out uwu
  // (we start at 1 because entry 0 is already filled out)

  for (uint32 i = 1; i < NumEntries; i++) {

    if (Table->Ebx == 0) {

      return i;

    }

    Table->Eax = 0xE820;
    Table->Ecx = 24;
    Table->Edx = 0x534D4150;

    Table->Di = (uint16)Buffer + (24*i);

    Table->Int = 0x15;

    realMode();

  }

  // If we've gotten to the end, then just return the total number of entries found

  return NumEntries - 1;

}
