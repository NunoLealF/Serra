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
static efiBlockIoProtocol* EfiBlockIoProtocols = NULL;

static uint64 NumEfiBlockIoHandles = 0;



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

  // Finally, now that we have a list of usable handles, let's fill out
  // `EfiBlockIoProtocols` by opening each corresponding protocol.

  // (Allocate enough space for `EfiBlockIoProtocols`; we can reuse
  // `HandleListSize` for this, since sizeof(efiHandle) is equal
  // to sizeof(efiProtocol))

  void* ProtocolList = Allocate(&HandleListSize);

  if (ProtocolList == NULL) {
    return false;
  } else {
    EfiBlockIoProtocols = (efiBlockIoProtocol*)ProtocolList;
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

  // Now that we're done, we can safely return `true`, to indicate
  // that everything went okay.

  return true;

}



// (TODO - Destructor: go through the list, and close protocols/handles
// for all. Returns `true` if successful, `false` if not.)

bool TerminateDiskSubsystem_Efi(void) {

  // (Before we do anything else, check if `EfiBlockIoHandles` is valid)

  if (EfiBlockIoHandles == NULL) {
    return false;
  }

  // (Close each instance of efiBlockIoProtocol, by manually going through
  // each member of `EfiBlockIoHandles`)

  for (uint64 Index = 0; Index < NumEfiBlockIoHandles; Index++) {

    gBS->CloseProtocol(EfiBlockIoHandles[Index],
                       &efiBlockIoProtocol_Uuid, ImageHandle, NULL);

  }

  // (Return `true`, now that we're done)

  return true;

}
