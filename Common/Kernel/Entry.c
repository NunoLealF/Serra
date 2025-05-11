// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "../InfoTable.h"

/* void Entrypoint()

   Inputs: kernelInfoTable* InfoTable - A pointer to the bootloader-provided kernel
           info table.

   Outputs: (None)

   TODO - ...

*/

typedef uint8 char8;
typedef uint16 char16;
#include "../../Boot/Efi/Efi/Efi.h"

efiSystemTable* gST;

void Entrypoint(kernelInfoTable* InfoTable) {

  // [Test out EFI, still very incomplete]

  if (InfoTable->Firmware.IsEfi == true) {

    kernelEfiInfoTable* EfiTable = InfoTable->Firmware.EfiInfo.Table;
    gST = EfiTable->SystemTable.Ptr;

    gST->ConOut->SetAttribute(gST->ConOut, 0x0F); gST->ConOut->OutputString(gST->ConOut, u"Hi, this is kernel mode Serra! <3\n\r");
    gST->ConOut->SetAttribute(gST->ConOut, 0x3F); gST->ConOut->OutputString(gST->ConOut, u"May 11 2025");
    gST->ConOut->SetAttribute(gST->ConOut, 0x0F); gST->ConOut->OutputString(gST->ConOut, u"\n\r");

    return;

  }

  // [Test out BIOS, still very incomplete]

  /*

  uint32* Vesathing = (uint32*)(InfoTable->Graphics.Vesa.Framebuffer);

  for (int i = 0; i < (640*480); i++) {
    Vesathing[i] = i;
  }

  */

  uintptr ThingAddr = (uintptr)InfoTable->Graphics.VgaText.Framebuffer;

  uint16* Thing = (uint16*)ThingAddr;
  char* Thing2 = "Hi, this is kernel mode Serra! <3";
  char* Thing3 = "May 11 2025";

  int Position = 0;

  for (int a = 0; a < (80*3); a++) {
    Thing[a] = 0;
  }

  while (Thing2[Position] != '\0') {
    Thing[Position] = 0x0F00 + Thing2[Position];
    Position++;
  }

  Position = 0;

  while (Thing3[Position] != '\0') {
    Thing[Position + 80] = 0x3F00 + Thing3[Position];
    Position++;
  }

  // TODO: Set up environment

  // (0) Make it so that the stub can eventually return.
  // (1) Check for problems - is SSE not enabled, sanity-check things, etc.
  // (2) Set up basic IDT, panic handling, etc.
  // (3) Actually

  for(;;);

  // (Return?)

  return;

}
