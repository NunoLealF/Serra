// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#if !defined(__amd64__) && !defined(__x86_64__)
  #error "This code must be compiled with an x86-64 cross compiler."
#endif

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
#include "../../System/System.h"
#include "../Disk.h"
#include "Bios.h"

// (TODO - Include the Int 13h wrapper)
// (TODO - I mean okay it's here, but make it a function definition)

// (This should be a raw, (512+16)-byte binary that's designed to run at the
// location specified by `Int13Wrapper_Location`, and read RDI sectors,
// starting at the LBA in RSI, from the drive number in DL, to the
// location at `Int13Wrapper_Data`, while returning EFLAGS.)

// (Since this is a raw binary designed to run at a specific address, it
// can't be linked normally - instead, it's first compiled by nasm,
// and then manually included using #embed.)

// (That being said, since `Int13.bin` could be virtually anything, we also
// append an 8-byte signature to the end, as well as two 4-byte values that
// must match `Int13Wrapper_Data` and `Int13Wrapper_Location` respectively.)

// (TODO - The data)

#define Int13_Signature 0x3331496172726553

static const uint8 Int13_Wrapper[] = {
  #embed "Int13.bin" if_empty('\0') limit(528)
};



// (TODO - The function itself..?)

typedef uint16 (*Fn_Int13)(uint64 Lba, uint16 Sectors, uint8 DriveNumber);
static Fn_Int13 Call_Int13 = (Fn_Int13)Int13Wrapper_Location;



// (TODO - Verify (check signature) and set up the int 13h wrapper (copy
// to the right location in memory pretty much))

// True means it's fine, false means probably not and you should panic

[[nodiscard]] bool InitializeDiskSubsystem_Bios(void) {

  // First, let's check to see if the first byte is zero, (and if so,
  // return false).

  if (Int13_Wrapper[0] == '\0') {
    return false;
  }

  // Next, let's check to see if the data at the end (the signature,
  // and the intended code and data addresses) is valid.

  const uint64* Signature = (const uint64*)((uintptr)Int13_Wrapper + 512);
  const uint32* IntendedData = (const uint32*)((uintptr)Int13_Wrapper + 512 + 8);
  const uint32* IntendedLocation = (const uint32*)((uintptr)Int13_Wrapper + 512 + 8 + 4);

  if (*Signature != Int13_Signature) {
    return false;
  } else if (*IntendedData != Int13Wrapper_Data) {
    return false;
  } else if (*IntendedLocation != Int13Wrapper_Location) {
    return false;
  }

  // Additionally, let's make some additional sanity checks, since we
  // need `Int13Wrapper_Data` and `Int13Wrapper_Location` to be at
  // sane locations

  // (Check that none of them overlap with the IVT, and that the
  // code is also below 7C00h)

  if (Int13Wrapper_Data < 0x400) {
    return false;
  } else if (Int13Wrapper_Location < 0x400) {
    return false;
  } else if (Int13Wrapper_Location >= 0x7C00) {
    return false;
  }

  // (Check that the data address is aligned to a 64 KiB boundary,
  // and that the location doesn't overlap with it)

  if ((Int13Wrapper_Data % 0x10000) != 0) {
    return false;
  }

  if (Int13Wrapper_Location >= Int13Wrapper_Data) {

    if (Int13Wrapper_Location < (Int13Wrapper_Data + 0xFE00)) {
      return false;
    }

  }

  // Finally, now that we know Int13_Wrapper[] is *probably* real code
  // that does what we want, let's copy the first 512 bytes to the
  // location specified by `Int13Wrapper_Location`.

  // (This doesn't include the variables we just checked, but that's
  // on purpose, since it would otherwise take up space)

  Memcpy((void*)Int13Wrapper_Location, (const void*)Int13_Wrapper, 512);

  // (Now that we're done, let's return `true`)

  return true;

}



// (TODO - Include a function to interface with int 13h); this will require
// a real mode stub that's (okay i just implemented it lmao))

[[nodiscard]] bool ReadDisk_Bios(void* Pointer, uint64 Lba, uint64 Sectors, uint8 DriveNumber) {

  // Before we do anything else, let's check to see if the disk subsystem
  // has been initialized yet (and if int 13h is available).

  if (DiskInfo.IsEnabled == false) {
    return false;
  } else if (DiskInfo.BootMethod != BootMethod_Int13) {
    return false;
  }

  // Next, let's verify that the values we were given are sane, and
  // make sure that we aren't reading past the end of the disk.

  if (Pointer == NULL) {
    return false;
  } else if (Sectors == 0) {
    return false;
  } else if ((Lba + Sectors) > DiskInfo.Int13.NumSectors) {
    return false;
  }

  // Now that we know that int 13h is available, that we can safely
  // use it, and that we're within bounds, we can start preparing
  // the necessary environment for it:

  // (1) Save the current state of the kernel, if applicable: interrupts
  // must be disabled, and the value in CR3 must be below 4 GiB. (TODO)

  // (2) Make sure that we can safely switch in and out of long mode.

  if (ReadFromControlRegister(3, false) >= 0xFFFF0000) {
    return false;
  }

  // (3) Calculate the amount of sectors we can safely read into our
  // data buffer, which is 63.5 KiB (FE00h bytes) long.

  const uint64 SectorsPerRead = (0xFE00 / DiskInfo.Int13.BytesPerSector);

  // Finally, we can now safely call (int 13h, ah = 42h) as many times
  // as necessary.

  // (Since we can't load everything at once, and we can't access memory
  // above 1 MiB from real mode, we load as much as we can (to the buffer
  // at `Int13Wrapper_Data`), and then copy it to `Pointer`)

  uintptr Address = (uintptr)Pointer;
  bool Status = true;

  while (Sectors > 0) {

    // Calculate the amount of sectors we need to read for now.

    uint32 NumSectors = ((Sectors >= SectorsPerRead) ? SectorsPerRead : Sectors);
    uint64 Size = (NumSectors * DiskInfo.Int13.BytesPerSector);

    // Call the wrapper, and obtain the result - in this case, the higher
    // 8 bits represent the value of AH, whereas the lower 8 bits is
    // clear if the carry flag wasn't set.

    // (We want AH to be zero, and the carry flag to not have been set,
    // so we know it's successful if the return value is zero)

    uint16 Result = Call_Int13(Lba, NumSectors, DriveNumber);

    if (Result != 0) {

      Status = false;
      goto Cleanup;

    }

    // Next, let's copy the data from `Int13Wrapper_Data`, using Memcpy():

    Memcpy((void*)Address, (const void*)Int13Wrapper_Data, Size);

    // (Just in case Memcpy() is borked for some reason, compare the
    // data to make sure the copy didn't actually fail)

    uint64* Compare[2] = {(uint64*)Address, (uint64*)Int13Wrapper_Data};

    if (*(Compare[0]) != *(Compare[1])) {

      Status = false;
      goto Cleanup;

    }

    // Finally, now that we're done, let's move onto the next iteration
    // of the loop.

    Address += Size;

    Lba += NumSectors;
    Sectors -= NumSectors;

  }



  Cleanup:

  // (TODO - Something to restore state (enable interrupts? + other things))

  // The IDT is restored, but interrupts are not re-enabled for you, so
  // include a check to deal with that - also, the CR3 stuff



  // Finally, now that we're done, we can return `Status` (which should
  // be true for a *successful* read).

  return Status;

}



// (NOTE - The int 13h subsystem doesn't require a terminate/destructor
// function)
