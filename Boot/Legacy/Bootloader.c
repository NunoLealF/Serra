// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"

#include "Bootloader.h"
#include "Graphics/Graphics.h"
#include "Memory/Memory.h"
#include "Rm/Rm.h"

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


/* void __attribute__((noreturn)) Crash()

   Inputs:    uint16 Error - The error code to crash with
   Outputs:   (None)

   This function simply just crashes the system if needed (for example, if something important
   isn't present in the system, like the lack of a method to enable the A20 line).

   TODO - This isn't even remotely complete, I don't have string functions or anything yet, so it
   just resets the system without telling the user anything.

   Also, error codes:
   0:   (Invalid error code)
   1:   (Couldn't enable A20)
   ...: (Undefined error code)

*/

void __attribute__((noreturn)) Crash(uint16 Error) {

  if (Error) {
      __asm__("nop");
  } // This is just here to stop GCC from going 'hey you didn't use this'

  // Jump to the x86 reset vector using the null entry in the GDT (which should crash literally
  // any system)

  __asm__("ljmp $0x00, $0xFFFFFFF0");

  // If doing that somehow *doesn't* reset the system, just halt indefinitely until the user
  // manually resets the system themselves.

  for(;;) {
    __asm__("cli; hlt");
  }

}


/* void __attribute__((noreturn)) Bootloader()

   Inputs:    (none)
   Outputs:   (none)

   This is our second-stage bootloader's main function. We jump here after Init().

   Todo: Write a more concise description. (I feel like this is only really possible after actually
   finishing this part, lol)

*/

/*
   In terms of the structure of the memory map:

   00000-00500h: Reserved by BIOS
   00500-07C00h: Free, but used as stack in real-mode
   07C00-07E00h: First-stage bootloader and BPB. 16-bit real mode.

   07E00-0C000h: Second-stage bootloader. 32-bit protected mode.
   0C000-0CE00h: Assembly protected and real mode 'environment'.
   0CE00-0D000h: Data area for the above (real mode).
   0D000-0F000h: General data area. Accessible from within 16-bit real mode with ES=0.
   0F000-0FC00h: Empty, but 'reserved'/'loaded'. No idea what to do with this, maybe use as data?
   0FC00-0FD00h: Empty, reserved for A20 and generally just important data.
   0FD00-10000h: Empty, non-reserved. Stack smash protector(?)

   10000-20000h: Current stack. 64KiB in size.
   20000-80000h: Data, probably.
   80000-FFFFFh: Reserved!
   (QEMU reserves anything above 9FC00, probably because of the EBDA)
   (A0000-BFFFFh is always considered reserved as well, video mmeory.)
   (E0000-EFFFFh as well.)

*/

void __attribute__((noreturn)) Bootloader(void) {

  // We've finally made it to our second-stage bootloader. We're in 32-bit x86 protected mode with
  // the stack at 20000h in memory, and our bootloader between 7E00h and FC00h in memory.


  // At the moment, we can only reliably access up to the first MiB of data (from 00000h to FFFFFh).
  // This is because we haven't yet enabled the A20 line, which is a holdover from the 8086 days.

  // Before having loaded our second-stage bootloader, our bootsector actually tried to enable it
  // using the BIOS function int 15h, ax 2401h. However, this isn't guaranteed to work on all
  // systems.

  // Before we try to use any methods to enable the A20 line, we want to check if it's already been
  // enabled (by the firmware, or by the BIOS function we executed earlier). If so, we want to skip
  // to the next part of the bootloader.

  bool A20EnabledByDefault = false;
  bool A20EnabledByKbd = false;
  bool A20EnabledByFast = false;

  if (CheckA20() == true) {

    // If the output of the CheckA20 function is true, then that means that the A20 line has
    // already been enabled.

    A20EnabledByDefault = true;

  } else {

    // If the output of the CheckA20 function is false, then that means that the A20 line has not
    // already been enabled.

    // In this case, we want to try out two methods to enable the A20 line, the first of which
    // involves the 8042 keyboard controller.

    EnableKbdA20();
    WaitA20();

    if (CheckA20() == true) {

      A20EnabledByKbd = true;

    } else {

      // If the first method didn't work, there's also a second method that works on some systems
      // called 'fast A20'.
      // This may crash the system, but we'll have to reset if we can't enable A20 anyways.

      EnableFastA20();
      WaitA20();

      if (CheckA20() == true) {

        A20EnabledByFast = true;

      } else {

        // At this point, we've exhausted all of the most common methods for enabling A20 (such
        // as the aforementioned BIOS interrupt, the 8042 keyboard controller method, and the
        // fast A20 method.

        // As it's necessary for us to enable the A20 line, we'll need to crash the system /
        // give out an error code if we get to this point.

        Crash(0x01);

      }

    }

  }

  // Terminal test

  InitializeTerminal(80, 25, 0xB8000);
  ClearTerminal();

  // Test e820
  // Max 128 entries, D000h to DC00h.

  void* Mmap = 0xD000;
  uint8 MmapEntries = 0;

  uint32 Continuation = 0;

  do {

    void* Entry = (0xD000 + (MmapEntries * 24));
    Continuation = GetMmapEntry(Entry, 24, Continuation);

    if (Continuation != 0) MmapEntries++;
    if (MmapEntries >= 5) break;

  } while (Continuation != 0);

  // Display a message, a

  for (;;) {

    for (int i = 0x01; i <= 0x0F; i++) {

      InitializeTerminal(80, 25, 0xB8000);
      Print("Hi, this is Serra!\n", i);
      Print("October 15th 2023\n\n", 0x0F);

      if (CheckA20() == true) {
        Print("A20 is enabled, hooray!\n\n", 0x0A);
      } else {
        Print("A20 is disabled. :(\n\n", 0x0C);
      }

      char Buffer[64];

      Print("Number of E820 entries: ", 0x0F);
      Print(Itoa(MmapEntries, Buffer, 10), 0x0B);

      Print("\nBase: ", 0x0F);
      Print(Itoa(*(uint32*)(Mmap+4), Buffer, 16), 0x0B);
      Print(Itoa(*(uint32*)(Mmap+0), Buffer, 16), 0x0B);

      Print("\nLimit: ", 0x0F);
      Print(Itoa(*(uint32*)(Mmap+12), Buffer, 16), 0x0B);
      Print(Itoa(*(uint32*)(Mmap+8), Buffer, 16), 0x0B);

      Print("\nFlags: ", 0x0F);
      Print(Itoa(*(uint32*)(Mmap+16), Buffer, 2), 0x0B);

      Print("\nAcpi: ", 0x0F);
      Print(Itoa(*(uint32*)(Mmap+20), Buffer, 2), 0x0B);

      for (int j = 0; j < 60000000; j++) {
        __asm__("nop");
      }

    }

  }

  // Things left to do:

  // A - Work on E820, memory map, etc.
  // B - Uhh, CPUID?

  for(;;);

}
