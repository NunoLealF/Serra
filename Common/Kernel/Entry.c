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

    gST = InfoTable->Firmware.Efi.Tables.SystemTable.Pointer;

    gST->ConOut->SetAttribute(gST->ConOut, 0x0F);
    gST->ConOut->OutputString(gST->ConOut, u"Hi, this is Serra! <3 ");

    gST->ConOut->SetAttribute(gST->ConOut, 0x3F);
    gST->ConOut->OutputString(gST->ConOut, u"May 15 2025");

    gST->ConOut->SetAttribute(gST->ConOut, 0x07);

    efiInputKey PhantomKey;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);
    return ((2UL << 32) | 0); // (2:0)

  } else {

    *(uint16*)0xB8280 = 0x0F + 'H';
    *(uint16*)0xB8282 = 0x0F + 'i';
    return ((2UL << 32) | 1); // (2:1)

  }


  // ...

  for(;;);
  return 0x000000000; // (0:0)

}
