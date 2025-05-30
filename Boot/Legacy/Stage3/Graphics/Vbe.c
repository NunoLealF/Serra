// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Graphics.h"

/* uint32 GetVbeInfoBlock()

   Inputs: volatile vbeInfoBlock* Buffer - A pointer to the buffer you want to store the info
           block in.

   Outputs: uint32 - The return value of the VBE function (the value in eax).

   This function uses the VBE BIOS function (int 10h, ah = 4Fh, al = 00h) to get the VBE info
   block structure (defined in Graphics.h as vbeInfoBlock{}), which contains useful
   information about the system's graphics capabilities.

   As with any other VBE function, you first want to check the return value, which is divided
   in two parts - the lowest 8 bits (al) are used to indicate whether the function is supported
   (al = 4Fh) or not (al != 4Fh), and the 8 bits after that are used to indicate the return
   status of the function (where 00h is successful).

   Because of that, this function can be used to check for VBE support - just use something
   like this to read the return value:

   -> (((GetVbeInfoBlock(&YourBufferHere) & 0xFF) == 0x4F) ? true : false;

*/

uint32 GetVbeInfoBlock(volatile vbeInfoBlock* Buffer) {

  // Prepare the info block itself

  Buffer->Signature = 0x32454256; // 'VBE2'; for 'VESA', change to 41534556h

  // Prepare the BIOS interrupt (int 10h, ah = 4Fh, al = 00h, es:di = Buffer)

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F00;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it (using our real-mode wrapper)

  RealMode();

  // If the signature checks out, return the value stored in eax; otherwise, return 0
  // to indicate that the table is invalid

  if (Buffer->Signature == 0x41534556) {
    return Table->Eax;
  } else {
    return 0;
  }

}


/* uint32 GetVbeModeInfo()

   Inputs: volatile vbeModeInfoBlock* Buffer - A pointer to the buffer you want to store the
           mode info block in.

           uint16 ModeNumber - The mode number of the VBE mode you want to get information from.

   Outputs: uint32 - The return value of the VBE function (the value in eax).

   This function calls the VBE BIOS function (int 10h, ah = 4Fh, al = 01h) to get a VBE
   mode info block structure (defined in Graphics.h as vbeModeInfoBlock{}), which contains
   useful information about a given video mode's capabilities.

   As with any other VBE function, this function returns 4Fh in the lower 8 bits if it's
   supported by the system, as well as a status code in the 8 bits above that, so please check
   for that first.

*/

uint32 GetVbeModeInfo(volatile vbeModeInfoBlock* Buffer, uint16 ModeNumber) {

  // Prepare the info block itself

  Buffer->ModeInfo.Reserved_Vbe1 = 1;
  Memset((void*)(int)&Buffer->Vbe2Info.Reserved_Vbe2, 0, 6);

  // Prepare the BIOS interrupt (int 10h, ah = 4Fh, al = 01h, cx = ModeNumber, es:di = Buffer)

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F01;

  Table->Ecx = ModeNumber;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it (using our real mode wrapper), and then return the value stored
  // in eax.

  RealMode();
  return Table->Eax;

}


/* uint32 SetVbeMode()

   Inputs: uint16 ModeNumber - The VBE mode number you want to set.

           bool UseCrtc - Whether to use the given CRTC settings (true) or not (false)

           bool UseLinearModel - Whether to use a regular (false) or linear (true) model

           bool ClearDisplay - Whether to clear the VRAM (true) or keep it as is (false)

           void* CrtcBuffer - A pointer to a CrtcInfoBlock{} structure (defined in the VESA VBE
           specification); this only matters if (UseCrtc == true) though

   Outputs: uint32 - The return value of the VBE function (the value in eax).

   This function calls the VBE BIOS function (int 10h, ah = 4Fh, al = 02h, bx = ModeNumber)
   in order to set a given video mode (specified in ModeNumber).

   As with any other VBE function, this function returns 4Fh in the lower 8 bits if it's
   supported by the system, as well as a status code in the 8 bits above that, so please check
   for that first.

*/

uint32 SetVbeMode(uint16 ModeNumber, bool UseCrtc, bool UseLinearModel, bool ClearDisplay, void* CrtcBuffer) {

  // Prepare the mode number and flags (which is stored in the same variable)

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

  // Next, prepare the BIOS interrupt (int 10h, ah = 4Fh, al = 02h, bx = ModeNumber,
  // es:di = CrtcBuffer)

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F02;

  Table->Ebx = ModeNumber;

  Table->Es = (uint16)((int)CrtcBuffer >> 4);
  Table->Di = (uint16)((int)CrtcBuffer & 0x0F);

  Table->Int = 0x10;

  // Finally, actually execute it (using our real mode wrapper), and then return the value
  // stored in eax.

  RealMode();
  return Table->Eax;

}


/* uint32 GetEdidInfoBlock()

   Inputs: volatile edidInfoBlock* Buffer - A pointer to the buffer you want to store the
           EDID info block in.

           uint16 ControllerNum - The EDID controller number (where 00h is the default, and
           available virtually everywhere that this function is supported)

   Outputs: uint32 - The return value of the VBE function (the value in eax).

   This function uses the (barely-documented) VBE BIOS function (int 10h, ah = 4Fh, al = 15h,
   bl = 01h) in order to obtain an EDID info block from the system (defined as edidInfoBlock{}
   in Graphics.h), which can tell us a lot of useful information (like the maximum supported
   resolution).

   As with any other VBE function, this function returns 4Fh in the lower 8 bits if it's
   supported by the system, as well as a status code in the 8 bits above that, so please check
   for that first.

*/

uint32 GetEdidInfoBlock(volatile edidInfoBlock* Buffer, uint16 ControllerNum) {

  // Prepare the BIOS interrupt (int 10h, ah = 4Fh, al = 15h, bl = 01h, cx = ControllerNum,
  // dx = (the EDID block number), es:di = Buffer)

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0x4F15;
  Table->Ebx = 0x01;

  Table->Ecx = ControllerNum;
  Table->Edx = 0x00;

  Table->Es = (uint16)((int)Buffer >> 4);
  Table->Di = (uint16)((int)Buffer & 0x0F);

  Table->Int = 0x10;

  // Actually execute it (using our real-mode wrapper)

  RealMode();

  // If the signature checks out, return the value stored in eax; otherwise, return 0
  // to indicate that the table is invalid

  if (Buffer->Signature == 0x00FFFFFFFFFFFF00) {
    return Table->Eax;
  } else {
    return 0;
  }

}


/* uint16 FindBestVbeMode()

   Inputs: uint16* VbeModeList - A (flat/non-segmented) pointer to the address specified in
           vbeInfoBlock{}->ModeInfo->VideoModeListPtr.

           uint16 PreferredX_Resolution - The preferred horizontal resolution, in pixels;
           this function will not return any video modes exceeding this

           uint16 PreferredY_Resolution - The preferred vertical resolution, in pixels;
           this function will not return any video modes exceeding this

   Outputs: uint16 - If successful, the VBE mode number of the best video mode that can be
   safely used in this system; otherwise, 0h or FFFFh.

   This function goes through every VBE mode (specified in VbeModeList) and tries to find
   the best mode for the system, using two criteria:

   -> First, it has to have the best color depth available (usually 16/24/32-bit color);

   -> Second, it has to have the largest resolution that doesn't exceed the preferred
   resolution (PreferredX_Resolution x PreferredY_Resolution pixels).

   This function requires VBE support in order to work, but unlike other VBE-related
   functions, it doesn't return a status code, so you cannot use this function just to check
   for VBE support.

*/

uint16 FindBestVbeMode(uint16* VbeModeList, uint16 PreferredX_Resolution, uint16 PreferredY_Resolution) {

  // Declare a few initial variables - the resolution of the current best mode, whether
  // 16-bit or 24/32-bit color is supported, a temporary mode info block, etc.

  uint16 BestVbeMode = 0;

  uint16 ModeX_Resolution = 0;
  uint16 ModeY_Resolution = 0;

  bool Supports16bColor = false;
  bool Supports24bColor = false;

  vbeModeInfoBlock VbeModeInfo;

  // Then, let's just go through each mode until we reach FFFFh (which indicates that we're
  // at the end of the list)

  while (*VbeModeList != 0xFFFF) {

    // First, get information about the current mode - we'll want to filter
    // out anything that doesn't have bit 0 (supported) and 7 (uses linear
    // framebuffer) set, as well as any non-direct (MemoryModel != 6) modes.

    uint32 VbeReturnStatus = GetVbeModeInfo(&VbeModeInfo, *VbeModeList);

    if (VbeReturnStatus != 0x004F) {

      VbeModeList++;
      continue;

    } else if ((VbeModeInfo.ModeAttributes & (1 << 7)) == 0) {

      VbeModeList++;
      continue;

    } else if ((VbeModeInfo.ModeAttributes & (1 << 0)) == 0) {

      VbeModeList++;
      continue;

    } else if (VbeModeInfo.ModeInfo.MemoryModel != 0x06) {

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
