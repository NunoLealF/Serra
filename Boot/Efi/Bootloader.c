// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

#if !defined(__amd64__) || !defined(__x86_64__)
  #error "This code must be compiled with an x86_64-elf cross-compiler"
#endif


/* efiSystemTable* gST, efiBootServices* gBS, efiRuntimeServices* gRT

   Definitions: (Efi/Efi.h, Efi/Tables.h)

   These are global EFI tables; they're initialized here, and can be used
   by any function that includes the Efi/Efi.h header file.

*/

efiSystemTable* gST;
efiBootServices* gBS;
efiRuntimeServices* gRT;


/* efiStatus (efiAbi SEfiBootloader)()

   Inputs: efiHandle ImageHandle - The firmware-provided image handle.

           efiSystemTable* SystemTable - A pointer to the firmware-provided
           system table.

   Outputs: (TODO: Either !EfiSuccess, or uses KernelInfoTable)

   [TODO] I can't write a proper description until this is done

*/

efiStatus (efiAbi SEfiBootloader) (efiHandle ImageHandle, efiSystemTable* SystemTable) {

  // First and foremost, we need to check to see if the tables our firmware
  // gave us are even valid. We can do this by checking their signature
  // and size, like this:

  // (Before everything, prepare a few global variables)

  KernelInfoTable.Firmware.IsEfi = true;
  KernelInfoTable.Firmware.EfiInfo.Address = (uint64)(&EfiInfoTable);
  EfiInfoTable.ImageHandle.Ptr = ImageHandle;

  // (Check the EFI System Table)

  if (SystemTable == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Signature != efiSystemTableSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Size < sizeof(efiSystemTable)) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Revision < calculateRevision(2, 0)) {
    return EfiUnsupported; // We don't support anything below UEFI 2.0
  }

  EfiInfoTable.SystemTable.Ptr = SystemTable;
  gST = SystemTable;

  // (Check the EFI Boot Services table)

  if (SystemTable->BootServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Signature != efiBootServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Size < sizeof(efiBootServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.BootServices.Ptr = SystemTable->BootServices;
  gBS = gST->BootServices;


  // (Check the EFI Runtime Services table)

  if (SystemTable->RuntimeServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Signature != efiRuntimeServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Size < sizeof(efiRuntimeServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.RuntimeServices.Ptr = SystemTable->RuntimeServices;
  gRT = gST->RuntimeServices;



  // Next, we also want to do the same thing, but for the input and output
  // protocols (SystemTable->ConIn and SystemTable->ConOut); it's not a
  // *deal breaker* if they aren't supported, we just want to make sure.

  // (Check for ConIn / efiSimpleTextInputProtocol)

  SupportsConIn = true;
  efiInputKey PhantomKey;

  if (gST->ConsoleInHandle == NULL) {
    SupportsConIn = false;
  } else if (gST->ConIn == NULL) {
    SupportsConIn = false;
  } else if (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiUnsupported) {
    SupportsConIn = false;
  } else {
    gST->ConIn->Reset(gST->ConIn, true);
  }

  // (Check for ConOut / efiSimpleTextOutputProtocol)

  SupportsConOut = true;
  char16* PhantomString = u" ";

  if (gST->ConsoleOutHandle == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut->Mode == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut->OutputString(gST->ConOut, PhantomString) == EfiUnsupported) {
    SupportsConOut = false;
  } else {
    gST->ConOut->Reset(gST->ConOut, true);
  }

  // (TODO: Switch to the best possible *text* mode, if conout is enabled.)

  Message(-1, u"Switch to the best possible text mode, TODO");

  // This works really similarly to how GOP does it; in fact QueryMode()
  // actually just returns columns and rows, so you could do that lol; just
  // make sure it's at least 80 and 25 (and then check by a*b)

  Message(Boot, u"Successfully initialized the console.");



  // (Check for GOP)

  Print(u"\n\r", 0);
  Message(Boot, u"Checking for GOP (Graphics Output Protocol) support.");

  efiGraphicsOutputProtocol* GopProtocol;
  efiUuid GopUuid = efiGraphicsOutputProtocol_Uuid;
  bool SupportsGop = false;

  if (gBS->LocateProtocol(&GopUuid, NULL, (void**)(&GopProtocol)) == EfiSuccess) {

    KernelInfoTable.Graphics.Type = Gop;
    SupportsGop = true;

    Message(Ok, u"Successfully located a GOP protocol instance using LocateProtocol().");
    Message(Info, u"Protocol instance is located at %xh", (uintptr)GopProtocol);

  } else {

    // If regular text mode isn't supported either, then just return.

    if (SupportsConOut == false) {
      return EfiUnsupported;
    } else {
      Message(Warning, u"Unable to initialize Graphics Output Protocol; using EFI text mode.");
    }

  }

  KernelInfoTable.Graphics.Type = ((SupportsGop == true) ? Gop : EfiText);



  // (Find the best text or graphics mode)

  uint32 GopMode = 0;

  if (SupportsGop == true) {

    Print(u"\n\r", 0);
    Message(Boot, u"Attempting to find the best video mode.");

    // TODO: Find graphics mode.

    // First, we need to find the max mode (mode numbers are contiguous, so
    // you can query modes 0 through (MaxMode-1))

    uint32 MaxMode = GopProtocol->Mode->MaxMode;

    // We want to find the mode with the highest resolution *and* color
    // depth, where (at least 640*480) > (better color) > (better resolution);

    // [additionally, ignore anything == PixelBltOnly]

    uint8 ColorDepth = 0;

    uint32 HorizontalResolution = 0;
    uint32 VerticalResolution = 0;

    for (uint32 Mode = 0; Mode < MaxMode; Mode++) {

      // Do the thing

      efiGraphicsOutputModeInformation* ModeInfo;
      uint64 Size;

      if (GopProtocol->QueryMode(GopProtocol, Mode, &Size, &ModeInfo) == EfiSuccess) {

        if (ModeInfo->PixelFormat < PixelBltOnly) {

          // If the color depth is too low (under 16-bits), skip.

          // (Check color depth)

          uint8 ModeColorDepth = 24;

          if (ModeInfo->PixelFormat == PixelBitMask) {

            ModeColorDepth = 32;

            // For each bit set in the reserved mask, decrement ModeColorDepth

            for (int Bit = 0; Bit < 32; Bit++) {

              if ((ModeInfo->PixelBitmask.ReservedMask & (1 << Bit)) != 0) {
                ModeColorDepth--;
              }

            }

          }

          // (Verify color depth)

          if (ModeColorDepth < 16) {

            Message(Warning, u"Skipping GOP mode %d (color depth is too low)");
            continue;

          }

          // Now that we know the color depth isn't too low, we check for
          // the following things, in order:

          // -> Whether this mode's resolution is at least 640 by 480;
          // -> Whether this mode's color depth is above or equal to the current maximum;
          // -> Whether this mode's resolution is above or equal to the current maximum;

          if ((ModeInfo->HorizontalResolution >= 640) && (ModeInfo->VerticalResolution >= 480)) {

            if (ModeColorDepth >= ColorDepth) {

              uint64 CombinedResolution = ((uint64)HorizontalResolution * (uint64)VerticalResolution);
              uint64 ModeCombinedResolution = ((uint64)ModeInfo->HorizontalResolution * (uint64)ModeInfo->VerticalResolution);

              if (ModeCombinedResolution >= CombinedResolution) {

                // If we found a better mode, then let's go

                GopMode = Mode;

                ColorDepth = ModeColorDepth;
                HorizontalResolution = ModeInfo->HorizontalResolution;
                VerticalResolution = ModeInfo->VerticalResolution;

              }

            }

          }

        } else {

          Message(Warning, u"Skipping GOP mode %d (unsupported pixel format %d).", Mode, ModeInfo->PixelFormat);
          continue;

        }

      }

    }

    // Show the mode

    Message(Ok, u"Found GOP mode %d, with a %d*%d resolution and %d-bit color depth.",
            GopMode, HorizontalResolution, VerticalResolution, ColorDepth);

  }



  // (TODO: Add memory-related functions, get the mmap, etc.)

  Print(u"\n\r", 0);
  Message(-1, u"TODO: Add memory-related functions, find mmap, etc.");



  // (TODO: Wait until user strikes a key, then return.)

  Print(u"\n\r", 0);
  if (SupportsConIn == true) {
    Message(Warning, u"Press any key to return.");
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);
  }

  return EfiSuccess;

}
