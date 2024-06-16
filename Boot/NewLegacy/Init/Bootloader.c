// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void __attribute__((noreturn)) Init()

   Inputs:    (none)
   Outputs:   (none)

   Initializes the segment registers (in this case, DS, ES, FS, GS, and SS) with the data selector
   (10h) as set in our GDT, sets the stack location to 20000h (where it'll have 64KiB of space),
   and then jumps to our Bootloader() function.

   This function is defined by our linker to be at 7E00h in memory, which is where our first-stage
   bootloader (our MBR) jumps to after loading our second-stage bootloader into memory and enabling
   protected mode.

   As this is essentially the first code we execute after enabling protected mode, we need to set
   up a basic environment for our code - specifically, we need to set up the segment registers
   (except for CS), and set up a basic stack, which in our case is at 20000h.

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

   Inputs:    (none)
   Outputs:   (none)

   This is our second-stage bootloader's main function. We jump here after Init().

   Todo: Write a more concise description. (I feel like this is only really possible after actually
   finishing this part, lol)

*/

// 7C00h -> 7E00h: 1st stage bootloader
// 7E00h -> 9E00h: 2nd stage bootloader
// 9E00h -> AC00h: Shared real-mode code
// AC00h -> AE00h: Shared real-mode data
// AE00h -> D000h: [Empty]
// D000h -> D800h: Completely empty IDT that shouldn't be used but that's still loaded by the rm functions
// D800h -> 10000h: [Empty again]
// 10000h -> 20000h: Stack
// 20000h -> ?????h: 3rd stage bootloader

// B000h ->
// F000h (4 bytes): A20 test location
//

void __attribute__((noreturn)) Bootloader(void) {

  // We've finally made it to our second-stage bootloader. We're in 32-bit x86 protected mode with
  // the stack at 20000h in memory, and our bootloader between 7E00h and CE00h in memory.


  // (Set up terminal)

  InitializeTerminal(80, 25, 0xB8000);
  ClearTerminal();

  Message(Kernel, "Entered second-stage bootloader");
  Message(Ok, "Successfully initialized the terminal (at B8000h)");


  // (Set up A20)

  // At the moment, we can only reliably access up to the first MiB of data (from 00000h to FFFFFh).
  // This is because we haven't yet enabled the A20 line, which is a holdover from the 8086 days.

  // Before having loaded our second-stage bootloader, our bootsector actually tried to enable it
  // using the BIOS function int 15h, ax 2401h. However, this isn't guaranteed to work on all
  // systems.

  // Before we try to use any methods to enable the A20 line, we want to check if it's already been
  // enabled (by the firmware, or by the BIOS function we executed earlier). If so, we want to skip
  // to the next part of the bootloader.

  bool A20_EnabledByDefault = false;
  bool A20_EnabledByKbd = false;
  bool A20_EnabledByFast = false;

  Putchar('\n', 0);
  Message(Kernel, "Preparing to enable the A20 line");

  if (Check_A20() == true) {

    // If the output of the CheckA20 function is true, then that means that the A20 line has
    // already been enabled.

    A20_EnabledByDefault = true;
    Message(Ok, "A20 line has already been enabled");

  } else {

    // If the output of the CheckA20 function is false, then that means that the A20 line has not
    // already been enabled.

    // In this case, we want to try out two methods to enable the A20 line, the first of which
    // involves the 8042 keyboard controller.

    Message(Kernel, "Attempting to enable the A20 line using the 8042 keyboard method");

    EnableKbd_A20();
    Wait_A20();

    if (Check_A20() == true) {

      A20_EnabledByKbd = true;
      Message(Ok, "The A20 line has successfully been enabled");

    } else {

      // If the first method didn't work, there's also a second method that works on some systems
      // called 'fast A20'.
      // This may crash the system, but we'll have to reset if we can't enable A20 anyways.

      Message(Fail, "The A20 line was not successfully enabled");
      Message(Kernel, "Attempting to enable the A20 line using the Fast A20 method");

      EnableFast_A20();
      Wait_A20();

      if (Check_A20() == true) {

        A20_EnabledByFast = true;
        Message(Ok, "The A20 line has successfully been enabled");

      } else {

        // At this point, we've exhausted all of the most common methods for enabling A20 (such
        // as the aforementioned BIOS interrupt, the 8042 keyboard controller method, and the
        // fast A20 method.

        // As it's necessary for us to enable the A20 line, we'll need to crash the system /
        // give out an error code if we get to this point.

        Panic("Failed to enable the A20 line");

      }

    }

  }

  // (Try to see if the real mode functions work)
  // (EDIT: They very much do! Enjoy the pretty lights show lol)

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = 0x13;
  Table->Int = 0x10;

  RealMode();

  for (uint16 i = 0; i < 320; i++) {

    for (uint16 j = 0; j < 200; j++) {

      Table->Eax = 0x0C00 + ((i+j) % 10) + j;
      Table->Ebx = 0;
      Table->Ecx = i;
      Table->Edx = j;

      Table->Int = 0x10;

      RealMode();

    }

  }



  // Right now, according to objdump, all of this code occupies about 3 KiB, so, about the
  // size of 6 sectors. We're limited to 16 sectors, and we still need to:

  // -> Read data from the disk, with (basic) FAT16/FAT32 support
  // -> Load the third stage bootloader into memory, and jump there.

  // For now, let's end things here.

  Putchar('\n', 0);

  Printf("Hi, this is Serra! <3\n", 0x0F);
  Printf("June %i %x\n", 0x3F, 16, 0x2024);

  for(;;);

}
