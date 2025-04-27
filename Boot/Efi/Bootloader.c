// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Efi/Efi.h"
#include "../../Common/InfoTable.h"

#if !defined(__amd64__) || !defined(__x86_64__)
  #error "This code must be compiled with an x86_64-elf cross-compiler"
#endif

// I know 'SEfiBootloader' sounds weird, but hear me out:
// -> S2Bootloader: Stage 2 (BIOS) bootloader
// -> S3Bootloader: Stage 3 (BIOS) bootloader
// -> SEfiBootloader: Stage EFI bootloader

// (...)

efiStatus efiAbi SEfiBootloader([[maybe_unused]] efiHandle ImageHandle, efiSystemTable* SystemTable) {

  // (Check signatures. If below EFI 1.1 or 2.0, go back..?)

  // Print something - we temporarily set the attribute (color code pretty much,
  // works *just* like VGA) to 0x0F instead of 0x07

  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0F);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, u"[SEfiBootloader] Hi sigmas <3 \n\r");
  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x07);

  // Try to find gop

  efiGraphicsOutputProtocol* GopInterface;
  efiUuid GopUuid = efiGraphicsOutputProtocol_Uuid;

  efiStatus Status = SystemTable->BootServices->LocateProtocol(&GopUuid, NULL, (void**)(&GopInterface));

  if (isSuccess(Status)) {

    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0A);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Yay, we found GOP\n\r");

  } else {

    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0C);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Couldn't find GOP :(\n\r");

  }

  // Halt, switch to regular text color, then return.

  for (uint64 i = 0; i < 5000000000; i++) {
    __asm__ __volatile__ ("nop");
  }

  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x07);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Okay, finished halting\n\r");

  return EfiSuccess;

}
