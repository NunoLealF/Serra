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

  #define InfoTable_Location 0xAE00
  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTable_Location);



  // (Initialize terminal table..)

  Debug = InfoTable->Debug;
  Memcpy(&TerminalTable, &InfoTable->Terminal_Info, sizeof(InfoTable->Terminal_Info));

  Putchar('\n', 0);
  Message(Kernel, "Successfully entered the third-stage bootloader.");



  // (Initialize IDT; entries are at 0xB000, or at what's set at IdtLocation)
  // Since the size is 2048 bytes, it ranges from B000h to B8000h

  // [TODO - **For the love of god, clean this up**]



  // (IDT section)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT.");

  descriptorTable* IdtDescriptor;
  IdtDescriptor->Size = (2048 - 1);
  IdtDescriptor->Offset = IdtLocation;

  // (PIC section)

  Message(Kernel, "Initializing the 8259 PIC.");

  MaskPic(0xFF); // Full mask, don't enable anything (set to 0xFE for timer, or 0xFD for keyboard that doesn't really work)
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  // (Initializing everything section)

  MakeDefaultIdtEntries(IdtDescriptor, 0x08, 0x0F, 0x00);
  LoadIdt(IdtDescriptor);

  __asm__("sti");
  Message(Ok, "Successfully initialized the IDT and PIC.");



  // [For now, let's just leave it here]

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("July %i %x\n", 0x3F, 28, 0x2024);

  for(;;);

}
