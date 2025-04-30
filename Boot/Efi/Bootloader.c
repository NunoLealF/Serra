// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

/* efiSystemTable* gST, efiBootServices* gBS, efiRuntimeServices* gRT

   Definitions: (Efi/Efi.h, Efi/Tables.h)

   These are global pointers to their respective EFI tables; they're
   declared here, initialized in SEfiBootloader(), and can be used
   by *any* function that uses the Efi/Efi.h header.

*/

efiSystemTable* gST;
efiBootServices* gBS;
efiRuntimeServices* gRT;


/* efiStatus efiAbi SEfiBootloader()

   Inputs: efiHandle ImageHandle - The firmware-provided image handle.

           efiSystemTable* SystemTable - A pointer to the firmware-provided
           system table.

   Outputs: ???

   TODO - ...

*/

efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable) {

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
  } else if (SystemTable->Hdr.Revision < calculateRevision(1, 1)) {
    return EfiUnsupported; // We don't support anything below EFI 1.1
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



  // Now that we know whether we can use EFI's text mode (by checking
  // SupportsConOut), we want to automatically switch to the best
  // text mode available.

  int32 ConOutMode = 0;
  uint32 ConOutResolution[2] = {0};

  if (SupportsConOut == true) {

    // EFI mode numbers run between 0 and (MaxMode - 1), so, we first
    // need to look for MaxMode

    int32 MaxMode = gST->ConOut->Mode->MaxMode;

    for (int32 Mode = 0; Mode < MaxMode; Mode++) {

      // Query mode information; we want to filter for modes that are
      // at least 80*25 characters, and with the largest overall area.

      uint64 Columns, Rows;

      if (gST->ConOut->QueryMode(gST->ConOut, Mode, &Columns, &Rows) == EfiSuccess) {

        if ((Columns >= 80) && (Rows >= 25)) {

          // If this text mode is larger than 80*25 characters and has
          // a larger overall area, then select it as our best mode.

          if ((Columns * Rows) >= (ConOutResolution[0] * ConOutResolution[1])) {

            ConOutMode = Mode;

            ConOutResolution[0] = Columns;
            ConOutResolution[1] = Rows;

          }

        }

      }

    }

    // Now that we know the best text mode to use, let's switch
    // to it!

    gST->ConOut->SetMode(gST->ConOut, ConOutMode);

  }

  // (Now that we've set up the console, show a few initial messages)

  Message(Boot, u"Successfully initialized the console.");

  Message(Info, u"This system's EFI revision is %d.%d", (uint16)(gST->Hdr.Revision >> 16), (uint16)(gST->Hdr.Revision));
  Message(Info, u"Using text mode %d, with a %d*%d character resolution", ConOutMode, ConOutResolution[0], ConOutResolution[1]);



  // Next, we also want to check for graphics mode support, since not all
  // systems support EFI text mode.

  // Thankfully, this is actually pretty easy; we just need to check for
  // the existence of the Graphics Output Protocol (GOP), like this:

  Print(u"\n\r", 0);
  Message(Boot, u"Checking for UEFI GOP (Graphics Output Protocol) support.");

  efiGraphicsOutputProtocol* GopProtocol;
  efiUuid GopUuid = efiGraphicsOutputProtocol_Uuid;
  bool SupportsGop = false;

  if (gBS->LocateProtocol(&GopUuid, NULL, (void**)(&GopProtocol)) == EfiSuccess) {

    KernelInfoTable.Graphics.Type = Gop;
    SupportsGop = true;

    Message(Ok, u"Successfully located a GOP protocol instance using LocateProtocol().");
    Message(Info, u"Protocol instance is located at %xh", (uintptr)GopProtocol);

  } else {

    // (If we didn't find one, that's fine, as long as text mode is
    // supported; otherwise, return)

    if (SupportsConOut == true) {
      Message(Warning, u"Unable to initialize GOP; keeping EFI text mode.");
    } else {
      return EfiUnsupported;
    }

  }

  // (Update the graphics type in the kernel info table)

  KernelInfoTable.Graphics.Type = ((SupportsGop == true) ? Gop : EfiText);



  // If the firmware *does* support GOP, then the next thing we want
  // to do is try to find the best graphics mode (that fits our
  // criteria, at least).

  uint32 GopMode = 0;

  uint8 GopColorDepth = 0;
  uint32 GopResolution[2] = {0};

  if (SupportsGop == true) {

    Print(u"\n\r", 0);
    Message(Boot, u"Attempting to find the best graphics mode.");

    // Just like with EFI text mode, GOP graphics modes also run between
    // 0 and (MaxMode - 1), so we first need to look for that.

    uint32 MaxMode = GopProtocol->Mode->MaxMode;

    // Now that we know the mode limit, we can manually go through
    // each mode and try to find the best one, if possible.

    for (uint32 Mode = 0; Mode < MaxMode; Mode++) {

      efiGraphicsOutputModeInformation* ModeInfo;
      uint64 Size;

      if (GopProtocol->QueryMode(GopProtocol, Mode, &Size, &ModeInfo) == EfiSuccess) {

        // If the pixel format is PixelBltOnly, then that means it's
        // not a linear framebuffer, so make sure it's not that

        if (ModeInfo->PixelFormat < PixelBltOnly) {

          // (Check and verify color depth - must be at least 16 bpp)

          uint8 ColorDepth = 24;

          if (ModeInfo->PixelFormat == PixelBitMask) {

            // For each bit set in the reserved mask, decrement ModeColorDepth
            // This is essentially just (32 - popcount(ReservedMask))

            ColorDepth = 32;

            for (int Bit = 0; Bit < 32; Bit++) {

              if ((ModeInfo->PixelBitmask.ReservedMask & (1 << Bit)) != 0) {
                ColorDepth--;
              }

            }

          }

          if (ColorDepth < 16) {

            Message(Warning, u"Skipping GOP mode %d (color depth is too low)");
            continue;

          }

          // Now that we know the color depth isn't too low, we check for
          // the following things, in order:

          // (1) Whether this mode's resolution is at least 640 by 480;
          // (2) Whether this mode's color depth is above the current maximum;
          // (3) Whether this mode's width is above or equal to the current maximum;

          if ((ModeInfo->HorizontalResolution >= 640) && (ModeInfo->VerticalResolution >= 480)) {

            if (ColorDepth >= GopColorDepth) {

              // In this case, we actually want to filter modes by their
              // horizontal resolution, not necessarily their area

              if (ModeInfo->HorizontalResolution >= GopResolution[0]) {

                GopMode = Mode;

                GopColorDepth = ColorDepth;
                GopResolution[0] = ModeInfo->HorizontalResolution;
                GopResolution[1] = ModeInfo->VerticalResolution;

              }

            }

          }

        } else {

          Message(Warning, u"Skipping GOP mode %d (unsupported pixel format %d).", Mode, ModeInfo->PixelFormat);
          continue;

        }

      }

    }

    // (Show the mode number, as well as the resolution / color depth)

    Message(Ok, u"Found GOP mode %d, with a %d*%d resolution and %d-bit color depth.",
            GopMode, GopResolution[0], GopResolution[1], GopColorDepth);

  }



  // -------------------------------------------------------------------------------------------------------------------------

  // (TODO: Add memory-related functions, get the mmap, etc.)

  Print(u"\n\r", 0);
  Message(-1, u"TODO: Add memory-related functions, find mmap, etc.");



  // (TODO: Wait until user strikes a key, then return.)

  if (SupportsConIn == true) {
    Message(Warning, u"Press any key to return.");
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);
  }

  return EfiSuccess;

}
