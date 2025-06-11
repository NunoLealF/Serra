// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Memory/Memory.h"
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

[[nodiscard]] bool InitializeDiskSubsystem(void* InfoTable) {

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



// (TODO - Function to actually read the data itself)

// The LBA must represent the real LBA, which means you need to add the
// partition offset to it)

[[nodiscard]] bool ReadSectors(void* Buffer, uint64 Lba, uint64 NumSectors, uint16 VolumeNum) {

  // (Read sectors using the driver indicated by Volume->Method)

  volumeInfo* Volume = &VolumeList[VolumeNum];

  if (Volume->Method == VolumeMethod_EfiBlockIo) {
    return ReadSectors_Efi(Buffer, Lba, NumSectors, Volume->Drive, Volume->MediaId);
  } else if (Volume->Method == VolumeMethod_Int13) {
    return ReadSectors_Bios(Buffer, Lba, NumSectors, Volume->Drive);
  }

  // (If we don't support this volume's method, then just return false;
  // the previous conditions should have returned already)

  return false;

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
  } else if (VolumeList[VolumeNum].Alignment >= (sizeof(uintptr) * 8)) {
    return false;
  } else if (VolumeList[VolumeNum].BytesPerSector == 0) {
    return false;
  } else if (VolumeList[VolumeNum].Method == VolumeMethod_Unknown) {
    return false;
  }

  // (Also, check the parameters we were given; if `Size` is zero, then
  // we don't need to read anything)

  volumeInfo* Volume = &VolumeList[VolumeNum];

  if (Buffer == NULL) {
    return false;
  } else if (Size == 0) {
    return true;
  }

  // Next, let's calculate the boundaries of where we'll need to read *from*,
  // and compare them against the overall volume boundaries.

  // (If the volume is a partition, add the necessary offset to `Offset`)

  if (Volume->IsPartition == true) {
    Offset += ((uint64)Volume->BytesPerSector * Volume->PartitionOffset);
  }

  // (Calculate the first and last *byte* we'll read)

  auto Start = Offset;
  auto End = (Start + Size - 1);

  // (Calculate the first and last *sector* we'll read)

  auto FirstSector = (Start / Volume->BytesPerSector);
  auto LastSector = (End / Volume->BytesPerSector);

  // (Make sure that `LastSector` doesn't exceed the total amount of
  // sectors on the drive)

  if (LastSector >= Volume->NumSectors) {
    return false;
  }

  // (Calculate the offsets we'll need to use, if we need to copy)

  auto StartOffset = (Start % Volume->BytesPerSector);
  auto EndOffset = ((End % Volume->BytesPerSector) + 1);

  // Although this depends on the system, some volumes require transfer
  // buffers to be aligned to a certain offset.

  // If `Buffer` doesn't meet these alignment requirements, then we
  // need to use a different strategy altogether, so before we do
  // anything else, let's check if our buffer is aligned:

  // (We also assume the buffer is misaligned for any reads that
  // don't have enough space to handle start *and* end)

  auto Alignment = (1ULL << Volume->Alignment);
  auto NumSectors = (1 + LastSector - FirstSector);

  bool BufferAligned = (((uintptr)Buffer % Alignment) == 0);

  if (NumSectors < 3) {
    BufferAligned = false;
  }

  if (BufferAligned == true) {

    // If we're here, that means our buffer address is properly aligned,
    // *so we can read data to it directly*.

    // However, that doesn't mean we can read everything in one go; if
    // our start or end positions aren't properly aligned, then we
    // need to deal with that first.

    // (Pre-calculate conditions)

    bool StartAligned = (StartOffset == 0);
    bool EndAligned = (EndOffset == Volume->BytesPerSector);

    // If the last sector isn't aligned, read it into the start of the
    // buffer, then move it to the correct position.

    if (EndAligned == false) {

      // (Read the last sector to the start of the buffer)

      bool Status = ReadSectors(Buffer, LastSector, 1, VolumeNum);

      if (Status == false) {
        return false;
      }

      // (*Copy* the data to the end of the buffer)

      uintptr Source = (uintptr)Buffer;
      uintptr Destination = (Source + Size - EndOffset);

      Memcpy((void*)Destination, (const void*)Source, EndOffset);

      // (Update `LastSector`, `NumSectors` and `Size`)

      LastSector--;
      NumSectors--;

    }

    // (Depending on whether the first sector is aligned, either read the
    // rest of it directly, or read our data in two parts)

    // In order to save space, we don't allocate a separate buffer, but
    // actually use the start of `Buffer`, since it's already aligned

    if (StartAligned == false) {

      // (Read all sectors, with the exception of the first sector, to
      // the beginning of the buffer)

      bool Status = ReadSectors(Buffer, (FirstSector + 1), (NumSectors - 1), VolumeNum);

      if (Status == false) {
        return false;
      }

      // (*Move* the data to where the start of the next sector would be)

      uintptr Source = (uintptr)Buffer;
      uintptr Destination = ((uintptr)Buffer + (Volume->BytesPerSector - StartOffset));

      Memmove((void*)Destination, (const void*)Source,
             (uint64)((NumSectors - 1) * Volume->BytesPerSector));

      // (Allocate a temporary buffer to store the first sector in)

      const uintptr TempSize = Volume->BytesPerSector;
      void* Temp = Allocate(&TempSize);

      if (Temp == NULL) {
        return false;
      }

      // (*Copy* the first `Volume->BytesPerSector` bytes of our transfer
      // buffer into `Temp`, to preserve it)

      Memcpy(Temp, Buffer, Volume->BytesPerSector);

      // (Read the first sector to the beginning of the buffer; this will
      // overflow into the remainder of our last read)

      Status = ReadSectors(Buffer, FirstSector, 1, VolumeNum);

      if (Status == false) {
        goto CleanupStart;
      }

      // (*Move* the first sector back by `StartOffset` bytes)

      Source = ((uintptr)Buffer + StartOffset);
      Destination = (uintptr)Buffer;

      Memmove((void*)Destination, (const void*)Source,
              (uint64)(Volume->BytesPerSector - StartOffset));

      // (*Copy* the remainder from `Temp` to our buffer; this restores
      // the data we overwrote earlier on)

      Source = ((uintptr)Temp + (Volume->BytesPerSector - StartOffset));
      Destination = ((uintptr)Buffer + (Volume->BytesPerSector - StartOffset));

      Memcpy((void*)Destination, (const void*)Source, StartOffset);

      // No matter what, we have to free the buffer we allocated, so
      // let's do just that:

      CleanupStart:

      if (Free(Temp, &TempSize) == true) {
        return Status;
      } else {
        return false;
      }

    } else {

      // (Read directly to the buffer)

      return ReadSectors(Buffer, FirstSector, NumSectors, VolumeNum);

    }

  } else {

    // If we're here, that means our buffer address is not aligned,
    // *so we can't read data to it directly*.

    // Instead, we'll need to allocate a temporary buffer to read
    // to, like this:

    const uintptr TempSize = Alignment + (NumSectors * Volume->BytesPerSector);
    void* TempPointer = Allocate(&TempSize);

    // (Make sure that the allocation succeeded, and declare a status
    // variable, so we can jump to `CleanupBuffer`)

    if (TempPointer == NULL) {
      return false;
    }

    bool Status = true;

    // Now that we have a buffer, we need to update the pointer so that
    // it also fits alignment requirements.

    // (`TempBuffer` is guaranteed to be aligned, but not `TempPointer`)

    uintptr TempAddress = (uintptr)TempPointer;

    if ((TempAddress % Alignment) > 0) {
      TempAddress += (Alignment - (TempAddress % Alignment));
    }

    void* TempBuffer = (void*)TempAddress;

    // Finally, now that we have a buffer we can read into, let's do that:
    // (We also want to check if the read was successful)

    Status = ReadSectors(TempBuffer, FirstSector, NumSectors, VolumeNum);

    if (Status == false) {
      goto CleanupBuffer;
    }

    // Now that we've read it, we can copy our transfer buffer's contents
    // to `Buffer`, using Memmove():

    TempBuffer = (void*)(TempAddress + StartOffset);

    Memmove(Buffer, TempBuffer, Size);
    goto CleanupBuffer;

    // (No matter what, we *have* to free the buffer we allocated)

    CleanupBuffer:

    if (Free(TempPointer, &TempSize) == true) {
      return Status;
    } else {
      return false;
    }

  }

}
