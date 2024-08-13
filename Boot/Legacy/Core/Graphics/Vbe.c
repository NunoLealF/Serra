// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "Graphics.h"

// A few things to keep in mind:
// -> *All VESA functions return 4Fh in AL if they are supported, and use AH as a status flag*
// -> The VESA/VBE specification is here: https://pdos.csail.mit.edu/6.828/2012/readings/hardware/vbe3.pdf


/*

(some important functions)

00h -> get VESA/VBE info, along with all video modes (query this first)
01h -> get mode info
02h -> set a video mode
03h -> get current mode

04h (?) -> save/restore state? hmm



*/




//
// (ax = 4F00h, es:di = location) is the most important one
// This is what it returns:

// ...





// ...

uint32 GetVbeInfoBlock(vbeInfoBlock* Buffer) {

  // Prepare the info block itself

  Buffer->Signature = 0x32454256; // 'VBE2'; for 'VESA', change to 41534556h

  // Prepare the BIOS interrupt

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F00;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Execute it

  RealMode();

  // If the signature checks out, return eax; otherwise..

  if (Buffer->Signature == 0x41534556) {
    return Table->Eax;
  } else {
    return 0;
  }

}
