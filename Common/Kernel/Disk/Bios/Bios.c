// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
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

#define Int13Wrapper_Signature 0x3331496172726553

const uint8 Int13Wrapper[] = {
  #embed "Int13.bin" if_empty('\0') limit(528)
};



// (TODO - The function itself..?)

typedef uint16 (*Fn_Int13Wrapper)(uint64 Lba, uint32 NumSectors, uint8 DriveNumber);
static Fn_Int13Wrapper Call_Int13Wrapper = (Fn_Int13Wrapper)Int13Wrapper_Location;



// (TODO - Verify (check signature) and set up the int 13h wrapper (copy
// to the right location in memory pretty much))

// True means it's fine, false means probably not and you should panic

[[nodiscard]] bool Setup_Int13Wrapper(void) {

  // First, let's check to see if the first byte is zero, (and if so,
  // return false).

  if (Int13Wrapper[0] == '\0') {
    return false;
  }

  // Next, let's check to see if the data at the end (the signature,
  // and the intended code and data addresses) is valid.

  const uint64* Signature = (const uint64*)((uintptr)Int13Wrapper + 512);
  const uint32* IntendedData = (const uint32*)((uintptr)Int13Wrapper + 512 + 8);
  const uint32* IntendedLocation = (const uint32*)((uintptr)Int13Wrapper + 512 + 8 + 4);

  if (*Signature != Int13Wrapper_Signature) {
    return false;
  } else if (*IntendedData != Int13Wrapper_Data) {
    return false;
  } else if (*IntendedLocation != Int13Wrapper_Location) {
    return false;
  }

  // Finally, now that we know Int13Wrapper[] is *probably* real code
  // that does what we want, let's copy the first 512 bytes to the
  // location specified by `Int13Wrapper_Location`.

  // (This doesn't include the variables we just checked, but that's
  // on purpose, since it would otherwise take up space)

  Memcpy((void*)Int13Wrapper_Location, (const void*)Int13Wrapper, 512);

  // (Now that we're done, let's return `true`)

  return true;

}



// (TODO - Include a function to interface with int 13h); this will require
// a real mode stub that's (okay i just implemented it lmao))

[[nodiscard]] bool Read_Int13Wrapper(void* Pointer, uint64 Lba, uint64 NumSectors, uint8 DriveNumber) {

  // (TODO - Sanity-check things, and declare initial values)

  uintptr Address = (uintptr)Pointer;

  if (Address == 0) {
    return false;
  } else if (DiskInfo.IsSupported == false) {
    return false;
  } else if (NumSectors == 0) {
    return false;
  }

  // (TODO - Something to save state (disable interrupts? + other things))

  // The IDT is saved, but you *need* to make sure CR3 can fit in the
  // lower 32-bits - *the original CR3 should be okay for this*

  // (TODO - Loop, since we can only do up to 127 sectors at a time; we
  // also need to memcpy as necessary)

  do {

    // (Call int 13h, and check the return value (it should be 0h for
    // a successful read - otherwise, return `false`)

    uint16 Result = Call_Int13Wrapper(Lba, (NumSectors % 127), DriveNumber);

    if (Result != 0) {
      return false;
    }

    // (Copy the data from `Int13Wrapper_Data`, using Memcpy())

    auto Remainder = (NumSectors % 127);
    auto Size = (Remainder * DiskInfo.Int13.BytesPerSector);

    Memcpy((void*)Address, (const void*)Int13Wrapper_Data, (uint64)Size);

    // (Move onto the next iteration of the loop)

    Address += Size;
    Lba += Remainder;
    NumSectors -= Remainder;

  } while (NumSectors > 127);

  // (TODO - Something to restore state (enable interrupts? + other things))

  // The IDT is restored, but interrupts are not re-enabled for you, so
  // include a check to deal with that - also, the CR3 stuff

  // (Now that we're done, we can return `true`)

  return true;

}
