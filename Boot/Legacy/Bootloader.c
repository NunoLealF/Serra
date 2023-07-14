// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void __attribute__((noreturn)) Init()

   Inputs: (none)
   Outputs: (none)

   Initializes the segment registers (in this case, DS, ES, FS, GS, and SS) with the data selector (10h) as
   set in our GDT, sets the stack location to 20000h (where it'll have 64KiB of space), and then jumps to our
   Bootloader() function.

   This function is defined by our linker to be at 7E00h in memory, which is where our first-stage bootloader
   (our MBR) jumps to after loading our second-stage bootloader into memory and enabling protected mode.

   As this is essentially the first code we execute after enabling protected mode, we need to set up a basic
   environment for our code - specifically, we need to set up the segment registers (except for CS), and
   set up a basic stack, which in our case is at 20000h.

*/

void __attribute__((noreturn)) Init(void) {

  // Set up registers (DS, ES, FS, GS, and SS)
  // We'll be setting these up with the data segment (10h in our GDT).

  __asm__("mov $0x10, %eax;"
          "mov %eax, %ds;"
          "mov %eax, %es;"
          "mov %eax, %fs;"
          "mov %eax, %gs;"
          "mov %eax, %ss");

  // Set up the stack at 20000h (by setting SP).

  __asm__("mov $0x20000, %esp");

  // Jump to our Bootloader() function.

  Bootloader();

}


/* void __attribute__((noreturn)) Bootloader()

   Inputs: (none)
   Outputs: (none)

   This is our second-stage bootloader's main function. We jump here after Init().

   Todo: Write a more concise description. (I feel like this is only really possible after
   actually finishing this part, lol)

*/

void __attribute__((noreturn)) Bootloader(void) {

  // Just test things out (this is just to see if this is actually working)

  char* testString = "Hi, this is Serra! <3\nJul 14 2023";

  int index = 0;
  int count = 0;

  for(;;) {
  for (int j = 0x1; j <= 0x0F; j++) {
  for (int i = 0; i < 34; i++) {
    if (testString[i] == '\n') {
      index = ((index / 80) + 1) * 80;
      count++;
      continue;
    }
    unsigned char color = j & 0x0F;
    if (count == 1) color = 0x0F;
    *(unsigned short*)(0xB8000 + (index*2)) = testString[i] | color << 8;
    index++;
  }
  index = 0; count = 0; // reset
  for (int k = 0; k < 50000000; k++) {
    __asm__("nop"); // this is to pause
  }
  }
  }

  // Things left to do:

  // A - Enable A20 line
  // B - Create a basic string library
  // C - Find a way to get -back- into real mode (see https://wiki.osdev.org/Real_Mode)
  // D - Use the above for E820, and other functions

  for(;;);

}
