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
// location specified by `Int13Wrapper_Location`, and read RDI sectors
// from the drive in RSI to the location at `Int13Wrapper_Data`.)

// (Since this is a raw binary designed to run at a specific address, it
// can't be linked normally - instead, it's first compiled by nasm,
// and then manually included using #embed.)

// (That being said, since `Int13.bin` could be virtually anything, we also
// append an 8-byte signature to the end, as well as two 4-byte values that
// must match `Int13Wrapper_Data` and `Int13Wrapper_Location` respectively.)

#define Int13Wrapper_Signature 0x3331496172726553

const uint8 Int13Wrapper[] = {
  #embed "Int13.bin" if_empty('\0') limit(528)
};



// (TODO - Verify (check signature) and set up the int 13h wrapper (copy
// to the right location in memory pretty much))

[[nodiscard]] bool Setup_Int13Wrapper(void) {

  // (Check if the first byte is zero, and if so, return false)

  if (Int13Wrapper[0] == '\0') {
    return false;
  }

  // (Next, let's check to see if the data at the end (the signature,
  // and the intended code and data addresses) are valid)

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

  // (Finally, now that we're done, let's return true)

  return true;

}



// (TODO - Include a function to interface with int 13h); this will require
// a real mode stub that's (...))

[[nodiscard]] bool Read_Int13Wrapper(void* Pointer, uint64 Lba, uint64 Size) {
  return false;
}
