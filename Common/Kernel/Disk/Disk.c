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

volumeInfo VolumeList[512] = {{0}};
uint16 NumVolumes = 0;



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

    // (Use InitializeDiskSubsystem_Efi() to initialize the EFI Boot
    // Services-dependent disk subsystem)

    bool Status = InitializeDiskSubsystem_Efi();

    if (Status == false) {
      return false;
    } else {
      DiskInfo.IsEnabled = true;
    }

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

    // (Use InitializeDiskSubsystem_Bios() to initialize the BIOS
    // int 13h-dependent disk subsystem)

    bool Status = InitializeDiskSubsystem_Bios();

    if (Status == false) {
      return false;
    } else {
      DiskInfo.IsEnabled = true;
    }

  } else {

    // If we don't have a boot method, then that means we don't know
    // how to interact with the disk, so let's return `false`

    return false;

  }

  // (Now that we're done, we can return true.)

  return true;

}



// (TODO - Destructor function; this is necessary for EFI!)

bool TerminateDiskSubsystem(void) {

  // Before we do anything else, let's check to see if the disk subsystem
  // has been enabled yet - and if not, disable it ourselves.

  if (DiskInfo.IsEnabled == false) {
    return false;
  }

  DiskInfo.IsEnabled = false;

  // Depending on the boot method, we may or may not need to manually
  // terminate the disk subsystem.

  // (BootMethod_Int13) Does not need to be terminated.
  // (BootMethod_Efi) Needs to be terminated, to close protocols.

  if (DiskInfo.BootMethod == BootMethod_Efi) {
    return TerminateDiskSubsystem_Efi();
  }

  return true;

}



// (TODO - Function to read data from a volume)

// This function works on a byte-by-byte basis (not block or sector size);
// as such, it *may* allocate extra memory or move the data to fit
// alignment or size requirements.

// (So, if you're making many large reads, try to at least keep `Buffer`
// aligned for performance reasons)

[[nodiscard]] bool ReadDisk(void* Buffer, uint64 Offset, uint64 Size, uint16 VolumeNum) {

  // Before we do anything else, we need to make sure that the disk subsystem
  // has been initialized, and that the given volume is usable.

  if (DiskInfo.IsEnabled == false) {
    return false;
  } else if (VolumeNum >= NumVolumes) {
    return false;
  } else if (VolumeList[VolumeNum].BytesPerSector == 0) {
    return false;
  } else if (VolumeList[VolumeNum].Method == VolumeMethod_Unknown) {
    return false;
  }

  // (Also, check the parameters we were given)

  if (Buffer == NULL) {
    return false;
  } else if (Size == 0) {
    return true;
  }

  // Next, let's calculate the boundaries of where we'll need to read *from*,
  // and compare them against the overall volume boundaries.

  // (Calculate the start and end blocks)

  volumeInfo* Volume = &VolumeList[VolumeNum];

  uint64 Start = Offset;
  uint64 End = (Offset + Size + Volume->BytesPerSector - 1);

  Start /= Volume->BytesPerSector;
  End /= Volume->BytesPerSector;

  // (Make sure they don't exceed the volume boundaries)

  if (End >= Volume->NumSectors) {
    return false;
  }

  // We also want to be able to deal with alignment constraints; more
  // specifically, we need to take into account:

  // -> (1) `Buffer` not being aligned to (Volume->Alignment), in which
  // case we need to either read twice, or allocate a new buffer;

  // -> (2) `Offset` being misaligned, in which case we need to



  /*

  Buffer OK, Offset (NOT OK)

  if buffer == OK, but offset != OK, then.. hmm


  [Size < BlockSize] Allocate buffer, then copy.

  [Size >= BlockSize] See table thing:

   B,O,S
  (Y,Y,Y) = Read directly
  (Y,Y,N) = Read twice (COMMON,e), then copy to last part of buffer
  (Y,N,Y) = Read twice (e,COMMON), then copy to first part of buffer
  (Y,N,N) = Read thrice (e,COMMON,e), then copy to edges of buffer

  (N,?,?) = Read once, then copy to buffer


  That truth table seems like it would be reasonable (in this case, OK
  means aligned); I wouldn't allocate a new buffer unless necessary though,
  often it's better to just do that sort of thing *inside* the buffer

  But there are exceptions; I think you have to find an aligned page within
  the buffer, and if you can't, only *then* allocate a page (and even then,
  you should be able to just

  In other terms:

  if (Size < BytesPerSector):
    *allocate a buffer from the stack, read into that, then copy*
  elif (Buffer % Alignment != 0):
    *allocate a buffer using Allocator(), read into that, then copy*
  else:
    *the truth table thing from earlier*

  */

}
