// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../../Common.h"
#include "Disk.h"

// (TODO 2 - The main point of this is to bring stuff from the common
// info table onto a shared table, and *later on* deal with
// initialization work and such)

// (TODO - Initialize global variables (in this case, `DiskInfo`))

diskInfo DiskInfo = {0};



// (TODO - Include a function to initialize the disk subsystem (?))

// P.S. - This will 100% be a stage 2 kernel environment thing, for now
// I *technically* don't need anything (other than InitializeCpuFeatures to
// make memory functions faster)

// But if in the future I ever want to implement anything other than what's
// provided by the firmware, I don't have much of a choice, I'll have to
// rely on PCI/PCIe/ACPI/even maybe timing

bool InitializeDiskSubsystem(void* InfoTable) {

  // Before we do anything else, let's check that the pointer we were
  // given actually matches a commonInfoTable{} structure.

  commonInfoTable* Table = (commonInfoTable*)InfoTable;

  if (Table->Signature != commonInfoTableSignature) {
    return false;
  }

  // What boot method did we use?

  switch (Table->Disk.Method) {

    case DiskMethod_Efi:
      DiskInfo.BootMethod = BootMethod_Efi;
      break;

    case DiskMethod_Int13:
      DiskInfo.BootMethod = BootMethod_Int13;
      break;

    default:
      DiskInfo.BootMethod = BootMethod_Unknown;
      break;

  }

  // (Depending on the specific boot method, fill out information)

  if (DiskInfo.BootMethod == BootMethod_Efi) {

    // (Set up the necessary fields in `DiskInfo`)

    //DiskInfo.Efi.

    // (TODO - Set up the environment)

  } else if (DiskInfo.BootMethod == BootMethod_Int13) {

    // ...

  } else {

    // If we don't have a boot method, then that means we don't know
    // how to interact with the disk, so..

    return false;

  }

  // (Now that we're done, we can return `true`)

  return true;

}
