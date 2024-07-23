// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void __attribute__((noreturn)) Bootloader()

   Inputs:    (none)
   Outputs:   (none)

   This is our third-stage bootloader's main function. We jump here after Init().

   Todo: Write a more concise description. (I feel like this is only really possible after actually
   finishing this part, lol)

*/

void __attribute__((noreturn)) Bootloader(void) {

  // TODO - a lot of things, but welcome to the third-stage bootloader!

  *(uint16*)0xB8000 = 'H' + (0x3F << 8);
  *(uint16*)0xB8002 = 'i' + (0x3F << 8);
  *(uint16*)0xB8004 = '!' + (0x3F << 8);

  // [For now, let's end things here]

  /*

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("July %i %x\n", 0x3F, 23, 0x2024);

  for(;;);

  */

  for(;;);

}
