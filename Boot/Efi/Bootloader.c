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


  // (TODO: Try enabling GOP)

  Message(Boot, u"Successfully booted into EFI application; this is purposefully long just to see if scrolling works it probably should I think.");
  Message(Warning, u"This should display *even* if Debug == false.");

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
