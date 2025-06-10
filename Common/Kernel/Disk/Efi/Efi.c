// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Firmware/Firmware.h"
#include "../../Memory/Memory.h"
#include "../Disk.h"
#include "Efi.h"

// TODO - I'm going to need a function to essentially:

// -> (1) Find the number of efiBlockIoProtocol handles, by calling
// gBS->LocateHandle() with an empty buffer

// -> (2) Allocate a buffer with the necessary size for that, using
// the memory management subsystem

// -> (3) Call `LocateHandle` again, this time actually getting a list
// of handles - one of them will match the boot one, so see if you
// can make that the first one


// Afterwards, you'll have to open protocols for each, which... should
// be fine? But there's a problem, which is that you have to close it
// too, so I think the entrypoint should take care of that

// (Or, have a destructor function that does that for you; I think that
// would make more sense, and be cleaner, imo.)




// (TODO - List of efiBlockIoProtocol handles, and protocols?)

static efiHandle* EfiBlockIoHandles = NULL;
static uintptr NumEfiBlockIoHandles = 0;

static efiBlockIoProtocol** EfiBlockIoProtocols = NULL;


// (TODO - Constructor: find efiBlockIoProtocol handles, open protocols
// for each of them, *add them to a list that can later be closed by
// the destructor* (EfiBlockIoProtocols!), return status)

[[nodiscard]] bool InitializeDiskSubsystem_Efi(void) {

  // Before we do anything else, let's make sure that both the EFI and
  // memory management subsystems are enabled.

  if (gST == NULL) {
    return false;
  } else if (gBS == NULL) {
    return false;
  }

  if (MmSubsystemData.IsEnabled == false) {
    return false;
  }

  // Okay: now that we know that those subsystems can be used, let's start
  // by querying the firmware for `efiBlockIoProtocol` handles.

  efiStatus Status = EfiSuccess;

  // (Find the necessary buffer size, by calling gBS->LocateHandle() with
  // an empty buffer - just in case, we allocate a temporary buffer)

  volatile uintptr Size = 0;
  char Temp[4096];

  Status = gBS->LocateHandle(ByProtocol, &efiBlockIoProtocol_Uuid, NULL,
                             (uintptr*)&Size, (efiHandle*)Temp);

  // (Make sure that the status returned by the firmware is sane, and
  // check that the firmware-returned buffer size isn't zero)

  if (Size == 0) {
    return false;
  } else if (Status != EfiBufferTooSmall) {
    return false;
  }

  // (Allocate a buffer with the size indicated by the firmware)

  const uintptr HandleListSize = Size;
  void* HandleList = Allocate(&HandleListSize);

  if (HandleList == NULL) {
    return false;
  }

  // (Point `EfiBlockIoHandles` to that buffer, and calculate the
  // number of efiBlockIoProtocol handles in it)

  EfiBlockIoHandles = (efiHandle*)HandleList;
  NumEfiBlockIoHandles = (HandleListSize / sizeof(efiHandle));

  if (NumEfiBlockIoHandles == 0) {
    return false;
  }

  // Now that we have a usable buffer, let's use gBS->LocateHandle() to
  // actually fill it out for us.

  Status = gBS->LocateHandle(ByProtocol, &efiBlockIoProtocol_Uuid, NULL,
                             (uintptr*)&Size, (efiHandle*)EfiBlockIoHandles);

  // (If the call didn't return successfully, return `false`)

  if (Status != EfiSuccess) {
    return false;
  }

  // Additionally, if the first handle doesn't match DiskInfo.Efi.Handle,
  // then let's iterate through each handle, and swap in the first one
  // that does - this should make the first handle the boot drive.

  if (EfiBlockIoHandles[0] != DiskInfo.Efi.Handle) {

    for (uint64 Index = 1; Index < NumEfiBlockIoHandles; Index++) {

      // (If we find a handle that matches `DiskInfo.Efi.Handle`, then
      // swap it with the first handle, and break)

      if (EfiBlockIoHandles[Index] == DiskInfo.Efi.Handle) {

        efiHandle SwapHandle = EfiBlockIoHandles[0];
        EfiBlockIoHandles[0] = EfiBlockIoHandles[Index];
        EfiBlockIoHandles[Index] = SwapHandle;

        break;

      }

    }

  }

  // Finally, now that we have a list of usable handles, let's fill out
  // `EfiBlockIoProtocols` by opening each corresponding protocol.

  // (Allocate enough space for `EfiBlockIoProtocols`)

  void* ProtocolList = Allocate(&HandleListSize);

  if (ProtocolList == NULL) {
    return false;
  } else {
    EfiBlockIoProtocols = (efiBlockIoProtocol**)ProtocolList;
  }

  // (Use gBS->OpenProtocol() on each handle in `EfiBlockIoHandles`;
  // later on, our destructor function will close these)

  for (uint64 Index = 0; Index < NumEfiBlockIoHandles; Index++) {

    // (Open an `efiBlockIoProtocol` instance at EfiBlockIoProtocols[n]
    // for each EfiBlockIoHandles[n])

    Status = gBS->OpenProtocol(EfiBlockIoHandles[Index],
                               &efiBlockIoProtocol_Uuid,
                               (void**)&EfiBlockIoProtocols[Index],
                               ImageHandle, NULL, 1);

    // (If there any issues, return `false`)

    if (Status != EfiSuccess) {
      return false;
    }

  }

  // Now that we're done, we can update `VolumeList` (from Disk.c)
  // to contain information about each instance, like this:

  auto VolumeLimit = (sizeof(VolumeList) / sizeof(volumeInfo));

  for (uint64 Index = 0; Index < NumEfiBlockIoHandles; Index++) {

    // (If we've exceeded the maximum amount of elements that
    // `VolumeList` can handle, then break)

    if (NumVolumes >= VolumeLimit) {
      break;
    }

    // (Obtain the protocol for this index, and make sure that it
    // was opened correctly, and is present in the system.)

    efiBlockIoProtocol* Protocol = EfiBlockIoProtocols[Index];

    if (Protocol == NULL) {
      continue;
    } else if (Protocol->Media == NULL) {
      continue;
    } else if (Protocol->Media->MediaPresent == false) {
      continue;
    }

    // (Create a volumeInfo{} structure, and fill it out)

    // In this case, the drive number corresponds to the index within
    // EfiBlockIoHandles[], and the partition number is always 0.

    volumeInfo* Volume = &VolumeList[NumVolumes];

    Volume->Method = VolumeMethod_EfiBlockIo;
    Volume->Drive = Index;
    Volume->Partition = 0;

    Volume->Type = VolumeType_Unknown; // (Should be filled by Fs.c later)
    Volume->MediaId = Protocol->Media->MediaId;
    Volume->Offset = 0;

    Volume->Alignment = 0;
    Volume->BytesPerSector = Protocol->Media->BlockSize;
    Volume->NumSectors = Protocol->Media->LastBlock;

    // (Calculate the alignment as a power of two - any transfer buffer
    // must be aligned to (1 << Volume->Alignment) == (Media->IoAlign))

    auto IoAlign = Protocol->Media->IoAlign;

    while ((IoAlign >> Volume->Alignment) > 1) {
      Volume->Alignment++;
    }

    // (Increment variables)

    NumVolumes++;

  }

  // Finally, now that we're done, let's return `true` to indicate
  // that everything went smoothly.

  return true;

}



// (TODO - Destructor: go through the list, and close protocols/handles
// for all. Returns `true` if successful, `false` if not.)

bool TerminateDiskSubsystem_Efi(void) {

  // (Before we do anything else, check if `EfiBlockIoHandles` is valid)

  if (EfiBlockIoHandles == NULL) {
    return false;
  }

  // Close each instance of efiBlockIoProtocol, by manually going through
  // each member of `EfiBlockIoHandles` ~ `EfiBlockIoProtocols`.

  for (uint64 Index = 0; Index < NumEfiBlockIoHandles; Index++) {

    // (Get this protocol's location and handle)

    efiHandle Handle = EfiBlockIoHandles[Index];
    efiBlockIoProtocol* Protocol = EfiBlockIoProtocols[Index];

    // (Flush all data from the device, if possible, before closing it)

    Protocol->FlushBlocks(Protocol);
    gBS->CloseProtocol(Handle, &efiBlockIoProtocol_Uuid, ImageHandle, NULL);

  }

  // (Return `true`, now that we're done)

  return true;

}



// (TODO - Function to read, like the ReadSectors_Bios() function)

[[nodiscard]] bool ReadSectors_Efi(void* Buffer, uint64 Lba, uint64 NumSectors, uint16 BlockIoIndex) {

  // (Make sure that the parameters we were given are valid)

  if (NumSectors == 0) {
    return false;
  } else if (Buffer == NULL) {
    return false;
  }

  // (Make sure that the disk subsystem has been initialized, that the
  // boot method is `BootMethod_Efi`, anduint64 that we're within bounds)

  if (DiskInfo.IsEnabled == false) {
    return false;
  } else if (DiskInfo.BootMethod != BootMethod_Efi) {
    return false;
  } else if (BlockIoIndex >= NumEfiBlockIoHandles) {
    return false;
  }

  // (Additionally, let's make sure that `EfiBlockIoProtocols` and
  // `EfiBlockIoProtocols[BlockIoIndex]` aren't null pointers.

  if (EfiBlockIoProtocols == NULL) {
    return false;
  } else if (EfiBlockIoProtocols[BlockIoIndex] == NULL) {
    return false;
  }

  // Now that we know we can probably use this protocol, let's see if
  // the device it represents is currently present, since not all
  // disks are non-removable (USB sticks, for example).

  efiBlockIoProtocol* Protocol = EfiBlockIoProtocols[BlockIoIndex];
  efiBlockIoMedia* Media = Protocol->Media;

  if (Media == NULL) {
    return false;
  } else if (Media->MediaPresent != true) {
    return false;
  }

  // (Also, make sure that the transfer buffer address is aligned to
  // Media->IoAlign; otherwise, the read could fail)

  if (Media->IoAlign > 1) {

    if (((uintptr)Buffer % (Media->IoAlign)) != 0) {
      return false;
    }

  }

  // Finally, now that we know we can probably read from the disk, let's
  // use the `ReadBlocks()` function to do just that.

  efiStatus Status;

  Status = Protocol->ReadBlocks(Protocol, Media->MediaId, (efiLba)Lba,
                                (NumSectors * Media->BlockSize),
                                (volatile void*)Buffer);

  // (Depending on the return status, either return `true` or `false`)

  if (Status != EfiSuccess) {
    return false;
  }

  return true;

}
