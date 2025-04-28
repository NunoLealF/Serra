// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

#if !defined(__amd64__) || !defined(__x86_64__)
  #error "This code must be compiled with an x86_64-elf cross-compiler"
#endif


/* efiStatus (efiAbi SEfiBootloader)()

   Inputs: efiHandle ImageHandle - The firmware-provided image handle.

           efiSystemTable* SystemTable - A pointer to the firmware-provided
           system table.

   Outputs: (TODO: Either !EfiSuccess, or uses KernelInfoTable)

   [TODO] I can't write a proper description until this is done

*/

// TODO: Add some sort of debug mechanism (symbol, maybe? config file??)
// (The Legacy bootloader just uses a variable in Bootsector.asm but that's
// obviously not portable lol.)

efiStatus (efiAbi SEfiBootloader) (efiHandle ImageHandle, efiSystemTable* SystemTable) {

  // First and foremost, we need to check to see if the tables our firmware
  // gave us are even valid. We can do this by checking their signature
  // and size, like this:

  // (Prepare kernel info table; unrelated to everything else)

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

  // (Check the EFI Boot Services table)

  if (SystemTable->BootServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Signature != efiBootServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Size < sizeof(efiBootServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.BootServices.Ptr = SystemTable->BootServices;

  // (Check the EFI Runtime Services table)

  if (SystemTable->RuntimeServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Signature != efiRuntimeServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Size < sizeof(efiRuntimeServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.RuntimeServices.Ptr = SystemTable->RuntimeServices;



  // Next, we also want to do the same thing, but for the input and output
  // protocols (SystemTable->ConIn and SystemTable->ConOut); it's not a
  // *deal breaker* if they aren't supported, we just want to make sure.

  // (Check for ConIn / efiSimpleTextInputProtocol)

  SupportsConIn = true;
  efiInputKey PhantomKey;

  if (SystemTable->ConsoleInHandle == NULL) {
    SupportsConIn = false;
  } else if (SystemTable->ConIn == NULL) {
    SupportsConIn = false;
  } else if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &PhantomKey) == EfiUnsupported) {
    SupportsConIn = false;
  } else {
    SystemTable->ConIn->Reset(SystemTable->ConIn, true);
  }

  // (Check for ConOut / efiSimpleTextOutputProtocol)

  SupportsConOut = true;
  char16* PhantomString = u" ";

  if (SystemTable->ConsoleOutHandle == NULL) {
    SupportsConOut = false;
  } else if (SystemTable->ConOut == NULL) {
    SupportsConOut = false;
  } else if (SystemTable->ConOut->OutputString(SystemTable->ConOut, PhantomString) == EfiUnsupported) {
    SupportsConOut = false;
  } else {
    SystemTable->ConOut->Reset(SystemTable->ConOut, true);
  }



  // (TODO: Test enabling GOP, and come up with a text mode graphics function;
  // in theory i can mostly just copy what's in Boot/Legacy/Shared/ tbf)

  // For now this just displays a message.

  if (SupportsConOut == true) {

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0B);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"You're at.. SEfiBootloader()\n\r");
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x07);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"TODO: You don't want to rely on this method forever, so make a proper function \n\r");

  }

  // Halt for a little while, and then return.

  for (uint64 i = 0; i < 5000000000; i++) {
    __asm__ __volatile__ ("nop");
  }

  return EfiSuccess;

}
