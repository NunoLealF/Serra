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

  // Print something - we temporarily set the attribute (color code pretty much,
  // works *just* like VGA) to 0x0F instead of 0x07

  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0F);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, u"[SEfiBootloader] Hi sigmas <3 \n\r");
  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x0B);

  // Wait for 5 seconds, printing the number each second
  // (For some reason O3 causes this to display "12222"?? lol what)

  char16* String = u"1";

  for (int i = 0; i < 5; i++) {

    SystemTable->ConOut->OutputString(SystemTable->ConOut, String);
    String[0]++;

    SystemTable->BootServices->RaiseTpl(TplHighLevel);
    SystemTable->BootServices->Stall(1000000);
    SystemTable->BootServices->RestoreTpl(TplApplication);

  }

  // Go to the next line, set attribute back to normal and then return.

  SystemTable->ConOut->SetAttribute(SystemTable->ConOut, 0x07);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, u"\n\r");

  return EfiSuccess;

}
