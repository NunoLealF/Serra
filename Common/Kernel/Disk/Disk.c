// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../../Common.h"
#include "Disk.h"

// (TODO 2 - The main point of this is to bring stuff from the common
// info table onto a shared table, and *later on* deal with
// initialization work and such)

// (TODO - Initialize global variables (`DiskInfo`, but probably others))

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

    DiskInfo.Efi.Handle = Table->Disk.Efi.Handle.Pointer;
    DiskInfo.Efi.Protocol = Table->Disk.Efi.Protocol.Pointer;

    DiskInfo.Efi.FileInfo = Table->Disk.Efi.FileInfo.Pointer;

    // (Make sure that they're valid - otherwise, return NULL)

    if (DiskInfo.Efi.FileInfo == NULL) {
      return false;
    } else if (DiskInfo.Efi.Handle == NULL) {
      return false;
    } else if (DiskInfo.Efi.Protocol == NULL) {
      return false;
    }

    // (TODO - Set up the environment)

    // ...

  } else if (DiskInfo.BootMethod == BootMethod_Int13) {

    // (Set up the necessary fields in `DiskInfo` - if EDD isn't enabled,
    // we use 'default' values for all fields but `DriveNumber`)

    DiskInfo.Int13.DriveNumber = Table->Disk.Int13.DriveNumber;
    DiskInfo.Int13.EddSupported = Table->Disk.Int13.Edd.IsSupported;

    if (DiskInfo.Int13.EddSupported == true) {

      DiskInfo.Int13.BytesPerSector = Table->Disk.Int13.Edd.BytesPerSector;
      DiskInfo.Int13.NumSectors = Table->Disk.Int13.Edd.NumSectors;

    } else {

      DiskInfo.Int13.BytesPerSector = 512;
      DiskInfo.Int13.NumSectors = uintmax;

    }

    // (Just in case, let's sanity-check the values we just added)

    if (DiskInfo.Int13.NumSectors == 0) {
      DiskInfo.Int13.NumSectors = uintmax;
    } else if (DiskInfo.Int13.BytesPerSector < 512) {
      return false;
    }

    // (Verify and set up the Int 13h wrapper - if this doesn't return
    // true, then that means there's probably something wrong)

    bool CanUseWrapper = Setup_Int13Wrapper();

    if (CanUseWrapper == false) {
      return false;
    }

    // (TODO - Set up the environment)

    // ...

  } else {

    // If we don't have a boot method, then that means we don't know
    // how to interact with the disk, so let's return `false`

    return false;

  }

  // (Now that we're done, we can return `true`)

  return true;

}
