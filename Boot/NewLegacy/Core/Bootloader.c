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

// 7C00h -> 7E00h: first stage bootloader, contains BPB, 0.5 KiB
// 7E00h -> 9E00h: second stage bootloader, 8 KiB
// 9E00h -> AC00h: real mode
// AC00h -> AE00h: real mode table
// AE00h -> B000h: info table
// B000h -> B800h: interrupt descriptor table
// B800h -> 10000h: (empty)
// 10000h -> 20000h: stack, 64 KiB
// 20000h -> 80000h: third stage bootloader, 384 KiB

void __attribute__((noreturn)) Bootloader(void) {

  // TODO - a lot of things, but welcome to the third-stage bootloader!

  // (Get info table..)

  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTable_Location);
  Debug = InfoTable->System_Info.Debug;

  // (Initialize terminal table..)

  Memcpy(&TerminalTable, &InfoTable->Terminal_Info, sizeof(InfoTable->Terminal_Info));

  Putchar('\n', 0);
  Message(Kernel, "Successfully entered the third-stage bootloader.");



  // (Prepare to initialize IDT and PIC)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT.");

  descriptorTable* IdtDescriptor;
  IdtDescriptor->Size = (2048 - 1);
  IdtDescriptor->Offset = IdtLocation; // Macro defined in Int/Int.h

  // (PIC section)

  Message(Kernel, "Initializing the 8259 PIC.");

  MaskPic(0xFF); // Full mask, don't enable anything (set to 0xFE for timer, or 0xFD for keyboard that doesn't really work)
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  // (Actually initialize the IDT)

  MakeDefaultIdtEntries(IdtDescriptor, 0x08, 0x0F, 0x00);
  LoadIdt(IdtDescriptor);

  __asm__("sti");
  Message(Ok, "Successfully initialized the IDT and PIC.");





  // (Set up A20)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to enable the A20 line");

  bool A20_EnabledByDefault = false;
  bool A20_EnabledByKbd = false;
  bool A20_EnabledByFast = false;

  if (Check_A20() == true) {

    // If the output of the CheckA20 function is true, then that means that the A20 line has
    // already been enabled.

    A20_EnabledByDefault = true;
    Message(Ok, "The A20 line has already been enabled.");

  } else {

    // If the output of the CheckA20 function is false, then that means that the A20 line has not
    // already been enabled.

    // In this case, we want to try out two methods to enable the A20 line, the first of which
    // involves the 8042 keyboard controller.

    Putchar('\n', 0);
    Message(Kernel, "Attempting to enable the A20 line using the 8042 keyboard method.");

    EnableKbd_A20();
    Wait_A20();

    if (Check_A20() == true) {

      A20_EnabledByKbd = true;
      Message(Ok, "The A20 line has successfully been enabled.");

    } else {

      // If the first method didn't work, there's also a second method that works on some systems
      // called 'fast A20'.
      // This may crash the system, but we'll have to reset if we can't enable A20 anyways.

      Message(Fail, "The A20 line was not successfully enabled.");

      Putchar('\n', 0);
      Message(Kernel, "Attempting to enable the A20 line using the fast A20 method.");

      EnableFast_A20();
      Wait_A20();

      if (Check_A20() == true) {

        A20_EnabledByFast = true;
        Message(Ok, "The A20 line has successfully been enabled.");

      } else {

        // At this point, we've exhausted all of the most common methods for enabling A20 (such
        // as the aforementioned BIOS interrupt, the 8042 keyboard controller method, and the
        // fast A20 method.

        // As it's necessary for us to enable the A20 line, we'll need to crash the system /
        // give out an error code if we get to this point.

        Panic("Failed to enable the A20 line.", 0);

      }

    }

  }





  // [For now, let's just leave things here]

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("July %i %x\n", 0x3F, 28, 0x2024);

  for(;;);

}
