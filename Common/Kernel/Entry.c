// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "../InfoTable.h"

/* uint64 Entrypoint()

   Inputs: commonInfoTable* InfoTable - A pointer to the bootloader-provided
           common information table.

   Outputs: uint64 - If the kernel can't be initialized for some reason,
            we return an error code (defined in TODO{}).

   TODO - ...

*/

#include "../../Boot/Efi/Efi/Efi.h"
efiSystemTable* gST;

uint64 Entrypoint(commonInfoTable* InfoTable) {


  // (TODO: Verify the table header)

  if (InfoTable->Signature != commonInfoTableSignature) {
    return ((1UL << 32) | 0); // (1:0)
  } else if (InfoTable->Version != commonInfoTableVersion) {
    return ((1UL << 32) | 1); // (1:1)
  } else if (InfoTable->Size != sizeof(commonInfoTable)) {
    return ((1UL << 32) | 2); // (1:2)
  }


  // (TODO: Verify the checksum)

  uint16 Checksum = 0;
  uint16 ChecksumSize = InfoTable->Size - sizeof(InfoTable->Checksum);
  uint8* RawInfoTable = (uint8*)InfoTable;

  for (uint16 Offset = 0; Offset < ChecksumSize; Offset++) {
    Checksum += RawInfoTable[Offset];
  }

  if (InfoTable->Checksum != Checksum) {
    return ((1UL << 32) | 3); // (1:3)
  }


  // (Demo)

  if (InfoTable->Firmware.Type == EfiFirmware) {

    if (InfoTable->System.Cpu.x64.ProtectionLevel == 0) __asm__ __volatile__ ("sti");

    gST = InfoTable->Firmware.Efi.Tables.SystemTable.Pointer;

    if (InfoTable->Display.Type == EfiTextDisplay) {

      gST->ConOut->SetAttribute(gST->ConOut, 0x0F);
      gST->ConOut->OutputString(gST->ConOut, u"Hi, this is Serra! <3 ");

      gST->ConOut->SetAttribute(gST->ConOut, 0x3F);
      gST->ConOut->OutputString(gST->ConOut, u"May 15 2025");

      gST->ConOut->SetAttribute(gST->ConOut, 0x07);

    } else {

      for (uint64 y = 0; y < InfoTable->Display.Graphics.LimitY; y++) {

        for (uint64 x = 0; x < (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 4); x++) {

          *(uint8*)(InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * y) + x) = (y * 255 / InfoTable->Display.Graphics.LimitY);

        }

      }

    }

    efiInputKey PhantomKey;
    if (InfoTable->Firmware.Efi.SupportsConIn == false) goto ReturnToEfi;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

    ReturnToEfi:

    if (InfoTable->System.Cpu.x64.ProtectionLevel == 0) __asm__ __volatile__ ("cli");
    return ((2UL << 32) | 0); // (2:0)

  } else {

    if (InfoTable->Display.Type == VgaDisplay) {

      char* Test = "Hi, this is Serra! <3";
      char* Test2 = "May 15 2025";

      uint16* TestPtr = (uint16*)(0xB8000 + (160*3));
      uint16* Test2Ptr = (uint16*)(0xB8000 + (160*4));

      for (uint16 a = 0; a < 160; a++) {
        TestPtr[a] = 0;
      }

      int Index = 0;
      while (Test[Index] != '\0') {
        TestPtr[Index] = 0x0F00 + Test[Index];
        Index++;
      }

      Index = 0;
      while (Test2[Index] != '\0') {
        Test2Ptr[Index] = 0x3F00 + Test2[Index];
        Index++;
      }

    } else {

      for (uint64 y = 0; y < InfoTable->Display.Graphics.LimitY; y++) {

        for (uint64 x = 0; x < (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 4); x++) {

          *(uint8*)(InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * y) + x) = (y * 255 / InfoTable->Display.Graphics.LimitY);

        }

      }

      for (uint64 a = 0; a < 100000000; a++) {
        __asm__ __volatile__ ("nop");
      }

    }

    return ((2UL << 32) | 1); // (2:1)

  }


  // ...

  for(;;);
  return 0; // (0:0)

}
