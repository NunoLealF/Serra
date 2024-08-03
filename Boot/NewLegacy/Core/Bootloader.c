// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void SaveState(), RestoreState()

   Inputs: (none)
   Outputs: (none)

   These two functions are called by Shared/Rm/RmWrapper.c, and they basically just save and
   restore the state of the system (for example, they might restore the IDT, paging, etc.)

   (TODO: actually comment this better, the specifics of what this is guaranteed to vary
   quite a bit)

*/

descriptorTable IdtDescriptor;

void SaveState(void) {

  __asm__ __volatile__ ("cli");

  return;

}

void RestoreState(void) {

  __asm__ __volatile__ ("sti");
  LoadIdt(&IdtDescriptor);

  return;

}


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

void Bootloader(void) {

  // (Get info table - check if signature is correct)

  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTable_Location);

  if (InfoTable->Signature != 0x65363231) {
    return;
  }

  // (Get info table, update debug)

  Debug = InfoTable->System_Info.Debug;



  // (Initialize terminal table..)

  Memcpy(&TerminalTable, &InfoTable->Terminal_Info, sizeof(InfoTable->Terminal_Info));

  Putchar('\n', 0);
  Message(Kernel, "Successfully entered the third-stage bootloader.");



  // (Prepare to initialize IDT and PIC)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT.");

  IdtDescriptor.Size = (2048 - 1);
  IdtDescriptor.Offset = IdtLocation;

  // (PIC section)

  Message(Kernel, "Initializing the 8259 PIC.");

  MaskPic(0xFF); // Full mask, don't enable anything (set to 0xFE for timer, or 0xFD for keyboard that doesn't really work)
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  // (Actually initialize the IDT)

  MakeDefaultIdtEntries(&IdtDescriptor, 0x08, 0x0F, 0x00);
  LoadIdt(&IdtDescriptor);

  __asm__("sti");
  Message(Ok, "Successfully initialized the IDT and PIC.");




  // (Set up A20)

  // This can only safely be done *after* initializing the IDT, because some methods aren't
  // really safe without one present (and also, they enable interrupts with sti)

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



  // [Get the *raw* E820 memory map]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to obtain the system's memory map, using E820");

  // First, we need to prepare, and check if it even works; each entry is 24 bytes, and
  // some systems can have a huge amount of entries, so we'll reserve space on the stack
  // (128 entries should be enough)

  #define MaxMmapEntries 128

  uint8 MmapBuffer[sizeof(mmapEntry) * MaxMmapEntries];
  mmapEntry* Mmap = (mmapEntry*)MmapBuffer;

  // Now, let's actually go ahead and fill that

  uint8 NumMmapEntries = 0;
  uint8 NumUsableMmapEntries = 0;

  uint32 MmapContinuation = 0;

  do {

    // Get the memory map entry, and save the continuation number in MmapContinuation

    MmapContinuation = GetMmapEntry((void*)&Mmap[NumMmapEntries], sizeof(mmapEntry), MmapContinuation);

    // Get the number of *usable* memory map entries

    if (Mmap[NumMmapEntries].Type == 1) {
      NumUsableMmapEntries++;
    }

    // Increment the number of entries, and break if we've reached the mmap entry limit

    NumMmapEntries++;

    if (NumMmapEntries >= MaxMmapEntries) {
      break;
    }

  } while (MmapContinuation != 0);


  // Show messages, if necessary

  if (NumMmapEntries > 0) {

    Message(Ok, "Successfully obtained the system memory map.");

    if (NumMmapEntries >= MaxMmapEntries) {
      Message(Warning, "Memory map entry limit exceeded; final map may be incomplete.");
    }

  } else {

    Panic("Failed to obtain the system's memory map.", 0);

  }




  // [Process the E820 memory map]

  // The first step here is to just sort each entry according to its base address,
  // like this (this is an O(n²) algorithm):

  for (uint16 Threshold = 1; Threshold < NumMmapEntries; Threshold++) {

    for (uint16 Position = Threshold; Position > 0; Position--) {

      if (Mmap[Position].Base < Mmap[Position - 1].Base) {
        Memswap((void*)(int)&Mmap[Position], (void*)(int)&Mmap[Position - 1], sizeof(mmapEntry));
      } else {
        break;
      }

    }

  }

  Message(Ok, "Successfully sorted the system memory map entries.");


  // [DEBUG]

  for (int a = 0; a < NumMmapEntries; a++) {
    Printf("%xh, %xh, %xh, %xh\n", 0xF3, (uint32)(Mmap[a].Base & 0xFFFFFFFF), (uint32)(Mmap[a].Limit & 0xFFFFFFFF), (uint32)(Mmap[a].Type), (uint32)(Mmap[a].Acpi));
  }

  // Next, we just need to isolate all the usable entries, and deal with overlapping
  // entries.

  uint8 UsableMmapBuffer[sizeof(mmapEntry) * NumUsableMmapEntries];
  mmapEntry* UsableMmap = (mmapEntry*)UsableMmap;

  uint64 MinStart = 0;
  uint8 UsablePosition = 0;

  for (uint8 Position = 0; Position < NumMmapEntries; Position++) {

    // Get the start and end position of this entry

    uint64 Start = Mmap[Position].Base;
    uint64 End = (Start + Mmap[Position].Limit);

    Printf("(regular) Start = %xh, End = %xh, Type = %xh, MINSTART = %xh\n", 0xF2, (uint32)(Start & 0xFFFFFFFF), (uint32)(End & 0xFFFFFFFF), Mmap[Position].Type, (uint32)(MinStart & 0xFFFFFFFF  ));

    // Next, update the minimum start, and skip over any entries that end before
    // it (if it's a usable entry, then decrement the number of usable entries)

    if (Start < MinStart) {
      Start = MinStart;
    }

    if (End <= MinStart) {

      if (Mmap[Position].Type == 1) {
        NumUsableMmapEntries--;
      }

      continue;

    }

    // Finally, if the type is right, then let's go ahead and actually add the entry
    // (if the type shows that it's usable)

    if (Mmap[Position].Type == 1) {

      // Update the start variable, if necessary

      if (Start < MinStart) {
        Start = MinStart;
      }

      // Update the end variable; for this, we only need to check for the next entry,
      // as the list of entries is already sorted for us

      if (Position < (NumMmapEntries - 1)) {

        if (End > Mmap[Position + 1].Base) {
          End = Mmap[Position + 1].Base;
        }

      }

      // Finally, add the entry

      UsableMmap[UsablePosition].Base = Start; Printf("Base: %xh, ", 0xF7, (uint32)(UsableMmap[UsablePosition].Base & 0xFFFFFFFF));
      UsableMmap[UsablePosition].Limit = (End - Start); Printf("Limit: %xh, End: %xh, ", 0xF7, (uint32)(UsableMmap[UsablePosition].Limit & 0xFFFFFFFF), (uint32)(End & 0xFFFFFFFF));
      UsableMmap[UsablePosition].Type = Mmap[Position].Type; Printf("Type: %xh, ", 0xF7, (uint32)UsableMmap[UsablePosition].Type);
      UsableMmap[UsablePosition].Acpi = Mmap[Position].Acpi; Printf("Acpi: %xh\n", 0xF7, (uint32)UsableMmap[UsablePosition].Acpi);

      UsablePosition++;

    }

    // Update MinStart

    MinStart = End;

  }


  Printf("Number of usable entries: %d\n", 0x7F, NumUsableMmapEntries);







  // [For now, let's just leave things here]

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("August %i %x\n", 0x3F, 3, 0x2024);

  for(;;);

}
