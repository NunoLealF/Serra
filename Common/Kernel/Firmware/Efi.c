// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../../Common.h"
#include "Firmware.h"

// Define global variables.
// (TODO - Document this properly)

efiSystemTable* gST = NULL;
efiBootServices* gBS = NULL;
efiRuntimeServices* gRT = NULL;

efiHandle ImageHandle = NULL;



// (TODO - A function to initialize global EFI variables (gST, gBS
// and gRT), as well as the image handle)

void InitializeEfiTables(void* InfoTable, efiSystemTable* SystemTable) {

  // (Make sure that `InfoTable` and `SystemTable` both have a valid
  // signature, and aren't null pointers)

  if (InfoTable == NULL) {
    return;
  } else if (SystemTable == NULL) {
    return;
  }

  commonInfoTable* Table = (commonInfoTable*)InfoTable;

  if (Table->Signature != commonInfoTableSignature) {
    return;
  } else if (SystemTable->Hdr.Signature != efiSystemTableSignature) {
    return;
  }

  // (Initialize gST to the address pointed to by `SystemTable`)

  gST = SystemTable;

  // (Initialize gBS and gRT to the Boot Services (gST->BootServices) and
  // Runtime Services (gST->RuntimeServices) tables, respectively)

  gBS = gST->BootServices;
  gRT = gST->RuntimeServices;

  // (Initialize the variable containing our EFI image handle, and return)

  ImageHandle = Table->Firmware.Efi.ImageHandle.Pointer;
  return;

}
