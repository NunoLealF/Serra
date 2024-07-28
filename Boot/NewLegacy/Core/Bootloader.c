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

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT.");

  descriptorTable* IdtDescriptor;
  IdtDescriptor->Size = (2048 - 1);
  IdtDescriptor->Offset = IdtLocation;

  MakeIdtEntry(IdtDescriptor, 0, (uint32)&IsrDivideFault, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 1, (uint32)&IsrDebug, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 2, (uint32)&IsrNmi, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 3, (uint32)&IsrBreakpoint, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 4, (uint32)&IsrOverflow, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 5, (uint32)&IsrOutOfBounds, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 6, (uint32)&IsrInvalidOpcode, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 7, (uint32)&IsrDeviceFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 8, (uint32)&IsrDoubleFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 9, (uint32)&IsrCoprocessorOverrun, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 10, (uint32)&IsrInvalidTss, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 11, (uint32)&IsrSegmentFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 12, (uint32)&IsrStackFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 13, (uint32)&IsrGpFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 14, (uint32)&IsrPageFault, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 15, (uint32)&IsrReservedA, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 16, (uint32)&Isr87Fault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 17, (uint32)&IsrAlignCheck, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 18, (uint32)&IsrMachineCheck, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 19, (uint32)&IsrSimdFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 20, (uint32)&IsrVirtFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 21, (uint32)&IsrControlFault, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 22, (uint32)&IsrReservedB, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 23, (uint32)&IsrReservedC, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 24, (uint32)&IsrReservedD, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 25, (uint32)&IsrReservedE, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 26, (uint32)&IsrReservedF, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 27, (uint32)&IsrReservedG, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 28, (uint32)&IsrHypervisorFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 29, (uint32)&IsrVmmFault, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 30, (uint32)&IsrSecurityFault, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 31, (uint32)&IsrReservedH, 0x08, 0x0F, 0x00);

  // (Set up PIC/IRQs)

  MakeIdtEntry(IdtDescriptor, 32, (uint32)&IrqTimer, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 33, (uint32)&IrqKeyboard, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 34, (uint32)&IrqCascade, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 35, (uint32)&IrqCom2, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 36, (uint32)&IrqCom1, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 37, (uint32)&IrqLpt2, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 38, (uint32)&IrqFloppy, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 39, (uint32)&IrqLpt1, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 40, (uint32)&IrqCmos, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 41, (uint32)&IrqPeripheralA, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 42, (uint32)&IrqPeripheralB, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 43, (uint32)&IrqPeripheralC, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 44, (uint32)&IrqMouse, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 45, (uint32)&IrqFpu, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 46, (uint32)&IrqHddA, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 47, (uint32)&IrqHddB, 0x08, 0x0F, 0x00);

  Message(Kernel, "Initializing the 8259 PIC.");

  MaskPic(0xFF); // Full mask, don't enable anything
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  LoadIdt(IdtDescriptor);

  __asm__("sti");
  MaskPic(0xFF); // I've already tested it and it works but let's not enable anything for now
  Message(Ok, "Successfully initialized the IDT and PIC.");



  // [For now, let's just leave it here]

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("July %i %x\n", 0x3F, 28, 0x2024);

  for(;;);

}
