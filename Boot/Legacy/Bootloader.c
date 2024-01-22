// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
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
   2:   (E820 doesn't work, can't get memory map)
   ...: (Undefined error code)

*/

// TODO - get rid of this, replaced by Panic().

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
   0D000-0D800h: Protected mode IDT. Accessible from within 16-bit real mode with ES=0.
   0D800-0F000h: General data area. Accessible from within 16-bit real mode with ES=0.
   (E000 is our E820 mmap)
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

  // (Set up terminal)

  InitializeTerminal(80, 25, 0xB8000);
  ClearTerminal();

  Message(Kernel, "Entered second-stage bootloader");
  Message(Ok, "Successfully initialized the terminal (at B8000h)");

  // (Set up IDT)

  descriptorTable* IdtDescriptor;
  IdtDescriptor->Size = (2048 - 1);
  IdtDescriptor->Offset = 0xD000;

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT");

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

  MakeIdtEntry(IdtDescriptor, 32+0, (uint32)&IrqTimer, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+1, (uint32)&IrqKeyboard, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+2, (uint32)&IrqCascade, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+3, (uint32)&IrqCom2, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+4, (uint32)&IrqCom1, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+5, (uint32)&IrqLpt2, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+6, (uint32)&IrqFloppy, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+7, (uint32)&IrqLpt1, 0x08, 0x0F, 0x00);

  MakeIdtEntry(IdtDescriptor, 32+8, (uint32)&IrqCmos, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+9, (uint32)&IrqPeripheralA, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+10, (uint32)&IrqPeripheralB, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+11, (uint32)&IrqPeripheralC, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+12, (uint32)&IrqMouse, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+13, (uint32)&IrqFpu, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+14, (uint32)&IrqHddA, 0x08, 0x0F, 0x00);
  MakeIdtEntry(IdtDescriptor, 32+15, (uint32)&IrqHddB, 0x08, 0x0F, 0x00);

  Message(Kernel, "Initializing the 8259 PIC");

  MaskPic(0xFF); // Full mask, don't enable anything
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  Message(Kernel, "Initializing the IDT");

  LoadIdt(IdtDescriptor); // load the idt lol

  __asm__("sti");
  MaskPic(0xFF); // I've already tested it and it works but let's not enable anything for now
  Message(Ok, "Successfully initialized the IDT and PIC");


  // (Set up A20)

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

  Putchar('\n', 0);
  Message(Kernel, "Preparing to enable the A20 line");

  if (CheckA20() == true) {

    // If the output of the CheckA20 function is true, then that means that the A20 line has
    // already been enabled.

    A20EnabledByDefault = true;
    Message(Ok, "A20 line has already been enabled");

  } else {

    // If the output of the CheckA20 function is false, then that means that the A20 line has not
    // already been enabled.

    // In this case, we want to try out two methods to enable the A20 line, the first of which
    // involves the 8042 keyboard controller.

    Message(Kernel, "Attempting to enable the A20 line using the 8042 keyboard method");

    EnableKbdA20();
    WaitA20();

    if (CheckA20() == true) {

      A20EnabledByKbd = true;
      Message(Ok, "The A20 line has successfully been enabled");

    } else {

      // If the first method didn't work, there's also a second method that works on some systems
      // called 'fast A20'.
      // This may crash the system, but we'll have to reset if we can't enable A20 anyways.

      Message(Fail, "The A20 line was not successfully enabled");
      Message(Kernel, "Attempting to enable the A20 line using the Fast A20 method");

      EnableFastA20();
      WaitA20();

      if (CheckA20() == true) {

        A20EnabledByFast = true;
        Message(Ok, "The A20 line has successfully been enabled");

      } else {

        // At this point, we've exhausted all of the most common methods for enabling A20 (such
        // as the aforementioned BIOS interrupt, the 8042 keyboard controller method, and the
        // fast A20 method.

        // As it's necessary for us to enable the A20 line, we'll need to crash the system /
        // give out an error code if we get to this point.

        Panic("Failed to enable the A20 line", 0);

      }

    }

  }

  // (Set up E820 / memory map)

  // In order to figure out which memory areas we can use, and which areas we can't, we'll
  // need to obtain what's known as a memory map from our BIOS/firmware.

  // There are several methods of doing this, but the most common is via the BIOS interrupt call
  // int 15h / eax E820h. We won't be directly calling it from here though; instead, we have a
  // function that will do it for us (GetMmapEntry(), in Memory/Mmap.c).

  // However, before we can start, we'll need to take care of two things - first, we'll need to
  // figure out where to put our memory map (in our case, this is at E000h), and second, we'll
  // also need what's known as a 'continuation number', which will be used later.

  void* Mmap = (void*)0xE000;
  uint32 Continuation = 0;

  Putchar('\n', 0);
  Message(Kernel, "Preparing to obtain the system's memory map (using E820)");

  // Now that we've taken care of that, we can finally request our system's memory map. We have
  // to do this entry by entry, and we have a total limit of 128 entries.

  uint8 MmapEntries = 0;

  do {

    // In order to request a memory map entry from our system, we first need to calculate the
    // necessary offset in memory, so that we don't overwrite any existing entries.

    uint32 Offset = (MmapEntries * 24);

    // After that, we just need to call GetMmapEntry() with the right parameters.

    Continuation = GetMmapEntry((void*)((uint32)Mmap + Offset), 24, Continuation);

    // If the 'continuation number' reaches 0, or if we've already read more than 128 entries
    // from the system, stop the loop.

    if (Continuation != 0) MmapEntries++;

    if (MmapEntries >= 128) {
      break;
    }

  } while (Continuation != 0);

  // Finally, if we weren't able to get *any* memory map entries from the system, give out an
  // error code / crash the system, as it's necessary for us to have a memory map.

  if (MmapEntries == 0) {

    Panic("Unable to obtain any memory map entries (using E820)", 0);

  } else {

    Message(Ok, "Successfully obtained a memory map (using E820)");

    char Buffer[64];

    mmapEntry* Test = (mmapEntry*)0xE000;

    Print("\nNumber of E820 entries: ", 0x0F);
    Print(Itoa(MmapEntries, Buffer, 10), 0x0B);

    Print("\nBase (entry 0): ", 0x0F);
    Print(Itoa((uint32)Test->Base, Buffer, 16), 0x0B);

    Print("\nLimit (entry 0): ", 0x0F);
    Print(Itoa((uint32)Test->Limit, Buffer, 16), 0x0B);

    Print("\nFlags (entry 0): ", 0x0F);
    Print(Itoa(Test->Type, Buffer, 16), 0x0B);

    Print("\nAcpi (entry 0): ", 0x0F);
    Print(Itoa(Test->Acpi, Buffer, 2), 0x0B);
    Print("\n\n", 0);


  }

  // (Try to see if CPUID works)

  Message(Kernel, "Preparing to check whether CPUID is available");

  if (CheckCpuid() > 0) {

    Message(Ok, "CPUID appears to be available");

  } else {

    Panic("CPUID is not available", 0);

  }

  // ...

  Print("\nHi, this is Serra!\n", 0x3F);
  Print("January 22nd 2024\n", 0x07);

  for(;;);


  // Things left to do:

  // A - Revamp the thing above [WILL START AFTER THIS COMMIT]
  // B - Finish working on E820, memory map, etc. [ALMOST DONE]
  // C - Uhh, CPUID? [STARTING TO WORK ON THIS NOW]

  for(;;);

}
