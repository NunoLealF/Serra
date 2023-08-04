// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"

/* bool CheckA20()

   Inputs:    (None)
   Outputs:   bool - Whether the A20 line is enabled (true) or disabled (false)

   This function checks to see if the A20 line is enabled or disabled. It does this by writing a
   certain value (a 'magic number') to FC00h (an empty memory location), and comparing it to the
   same memory address + 1MiB (100000h).

   On some systems, the A20 line (which lets us reliably access memory over 1MiB) isn't accessible
   by default as to maintain compatibility with programs built for the 8086 (which couldn't access
   more than 20 memory lines).

   Because of this, any memory addresses using the A20 line (such as 10FC00h) would instead 'wrap
   around' to a memory address where the A20 line (the 21st bit) would be cleared (like FC00h).

   Therefore, by writing two different values to FC00h and 10FC00h (or just XXXXh and 10XXXXh in
   general) and comparing them, we can determine whether the A20 line has been enabled or not.

*/

bool CheckA20(void) {

  // Values like 00000000h or FFFFFFFFh might be already present on both memory addresses, so we
  // use a unique 'magic number' to do the comparison.

  uint32 magicNumber = 0x31323665;

  // Write our 'magic number' to FC00h (an empty memory address), and to the same address + 1MiB.

  volatile uint32* lowTestAddress = (uint32*)(0xFC00);
  volatile uint32* highTestAddress = (uint32*)(0xFC00 + 0x100000);

  *lowTestAddress = magicNumber;
  *highTestAddress = ~magicNumber;

  // Compare the two addresses.

  if (*lowTestAddress != *highTestAddress) {

    return true;

  }

  return false;

}


/* void WaitA20()

   Inputs:    (None)
   Outputs:   (None)

   On some systems, enabling the A20 line might not take effect immediately, which could cause
   problems if the bootloader can't detect if the A20 line was actually enabled on or not.

   This function solves that by executing 4096 nop instructions, which should (hopefully) give
   the firmware enough time to let any changes come into effect.

*/

void WaitA20(void) {

  for (int i = 0; i < 4096; i++) {

    __asm__("nop");

  }

}


// Methods of enabling A20; these aren't easy to do in inline Assembly, so they're in a separate
// A20_Asm.s file.

extern void EnableKbdA20(void);
extern void EnableFastA20(void);
