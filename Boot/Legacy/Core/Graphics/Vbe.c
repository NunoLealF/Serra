// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
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

// I should also do something to get vbe mode info
// First, the table







// Returns the value of (E)AX; first, check to see if it's 4Fh

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




// Something to get mode info; ax = 4F01h.

uint32 GetVbeModeInfo(vbeModeInfoBlock* Buffer, uint16 ModeNumber) {

  // Prepare the info block itself

  Buffer->ModeInfo.Reserved_Vbe1 = 1;
  Memset((void*)(int)&Buffer->Vbe2Info.Reserved_Vbe2, 0, 6);

  // Prepare the BIOS interrupt

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F01;

  Table->Ecx = ModeNumber;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it

  RealMode();
  return Table->Eax;

}





// Something to set the mode; ax = 4F02h.
// (Disclaimer: CrtcBuffer only matters if UseCrtc == true)

uint32 SetVbeMode(uint16 ModeNumber, bool UseCrtc, bool UseLinearModel, bool ClearDisplay, void* CrtcBuffer) {

  // Prepare the mode number

  ModeNumber &= 0x1FF; // Preserve bits 0 through 8

  if (UseCrtc == true) {
    ModeNumber |= (1 << 11);
  }

  if (UseLinearModel == true) {
    ModeNumber |= (1 << 14);
  }

  if (ClearDisplay == false) {
    ModeNumber |= (1 << 15);
  }

  // Prepare the BIOS interrupt

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F02;

  Table->Ebx = ModeNumber;

  Table->Es = (uint16)((int)CrtcBuffer >> 4);
  Table->Di = (uint16)((int)CrtcBuffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it

  RealMode();
  return Table->Eax;

}



// Something to get EDID data; ax = 4F15h, bl = 01h

uint32 GetEdidInfoBlock(edidInfoBlock* Buffer, uint16 ControllerNum) {

  // Prepare the BIOS interrupt

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F15;
  Table->Ebx = 0x01;

  Table->Ecx = ControllerNum;
  Table->Edx = 0;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it

  RealMode();

  // If the signature checks out, return eax; otherwise..

  if (Buffer->Signature == 0x00FFFFFFFFFFFF00) {
    return Table->Eax;
  } else {
    return 0;
  }

}
