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



// A function to find (and return) the best VBE mode

uint16 FindBestVbeMode(uint16* VbeModeList, uint16 PreferredX_Resolution, uint16 PreferredY_Resolution) {

  // Declare a few initial variables.

  uint16 BestVbeMode = 0;

  uint16 ModeX_Resolution = 0;
  uint16 ModeY_Resolution = 0;

  bool Supports16bColor = false;
  bool Supports24bColor = false;

  vbeModeInfoBlock VbeModeInfo;
  uint32 VbeReturnStatus;

  // Then, just.. go through the modes, I guess

  while (*VbeModeList != 0xFFFF) {

    // First, get information about the current mode

    VbeReturnStatus = GetVbeModeInfo(&VbeModeInfo, *VbeModeList);

    if (VbeReturnStatus != 0x004F) {

      VbeModeList++;
      continue;

    }

    // Next, let's analyze that information: first, is this the first time we see a
    // 16-bit or a 24/32-bit mode?

    uint16 BitsPerPixel = VbeModeInfo.ModeInfo.BitsPerPixel;
    uint16 X_Resolution = VbeModeInfo.ModeInfo.X_Resolution;
    uint16 Y_Resolution = VbeModeInfo.ModeInfo.Y_Resolution;

    bool FoundBestMode = false;

    if ((BitsPerPixel >= 24) && (Supports24bColor == false)) {

      FoundBestMode = true;
      Supports24bColor = true;

    } else if ((BitsPerPixel >= 16) && (Supports16bColor == false)) {

      FoundBestMode = true;
      Supports16bColor = true;

    }

    // And, second, if the color depth is 'up to our standards', then does it meet the
    // resolution criteria?

    if ((BitsPerPixel >= 24) && (Supports24bColor == true)) {

      if (X_Resolution > PreferredX_Resolution || Y_Resolution > PreferredY_Resolution) {
        FoundBestMode = false;
      } else if (X_Resolution < ModeX_Resolution || Y_Resolution < ModeY_Resolution) {
        FoundBestMode = false;
      } else {
        FoundBestMode = true;
      }

    } else if ((BitsPerPixel >= 16) && (Supports16bColor == true)) {

      if (X_Resolution > PreferredX_Resolution || Y_Resolution > PreferredY_Resolution) {
        FoundBestMode = false;
      } else if (X_Resolution < ModeX_Resolution || Y_Resolution < ModeY_Resolution) {
        FoundBestMode = false;
      } else {
        FoundBestMode = true;
      }

    }

    // Finally, we can now update the best mode, and continue the loop

    if (FoundBestMode == true) {

      BestVbeMode = *VbeModeList;
      ModeX_Resolution = X_Resolution;
      ModeY_Resolution = Y_Resolution;

    }

    VbeModeList++;

  }

  // Finally, we can return the answer; if we didn't find any mode, we'll return 0h, but
  // otherwise..

  return BestVbeMode;

}
