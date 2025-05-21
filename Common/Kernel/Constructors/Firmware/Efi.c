// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "Firmware.h"

// Define global variables.
// (TODO - Document this properly)

efiSystemTable* gST = NULL;
efiBootServices* gBS = NULL;
efiRuntimeServices* gRT = NULL;



// (TODO - A function to initialize global EFI variables (gST, gBS, gRT))

// [!] This function doesn't check for null pointers, please don't just
// call this blindly

uint64 InitializeEfiTables(efiSystemTable* SystemTable) {

  // Initialize gST to SystemTable.

  gST = SystemTable;

  // Initialize gBS and gRT to the Boot Services (gST->BootServices) and
  // Runtime Services (gST->RuntimeServices) tables, respectively.

  gBS = gST->BootServices;
  gRT = gST->RuntimeServices;

  // Return.

  return (uint64)(&gST);

}
