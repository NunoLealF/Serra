// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Shared/Stdint.h"
#include "../../../Shared/Rm/Rm.h"
#include "../Memory.h"

/* uint32 GetMmapEntry()

   Inputs: void* Buffer - The memory area/buffer you want to store your entry in.
           uint32 Size - The size of the memory map entry you want to request.
           uint32 Continuation - The 'continuation number'.

   Outputs: uint32 - The 'continuation number' returned by the BIOS/firmware.
            (void* Buffer) - If the function ran successfully, the given memory map entry.

   This function asks for a memory map entry from the system, using the BIOS interrupt call
   int 15h / eax E820h. This BIOS function returns a map of the memory/RAM in the system (which
   is what we call a 'Memory map' / 'Mmap'), one entry at a time.

   In order to do so, it needs three important variables - the area you want to write your
   entry to (Buffer), the size of the entry you want to request from the firmware (Size), and
   a 'continuation number' (Continuation), which is a number given to us by the BIOS that
   indicates whether we should continue calling this function or not.

   To properly make use of this function, you would do the following:

   - First, you'd prepare two things - a buffer (or other memory area) large enough to fit
   everything, and a 'continuation number' (which should be initially set to 0);

   - Second, you'd call this function using those two variables, and check for the continuation
   number;

   - Third, if the continuation number isn't 0, then you'd call this function again with the
   returned continuation number, and a new buffer where you want to put the next entry.
   - You'd repeat this until you ran out of space and/or the BIOS set the continuation number
     to 0, indicating that there are no more entries.

   In some cases, this function might not work, even on the first try. In that case, that means
   the BIOS function being used (int 15h / eax E820h) is *not* supported, which can sometimes
   happen on some older systems (generally from before 2002).

   Although there are no standards that explicitly specify *how* these entries should be
   formatted, you should generally specify a minimum of 20 bytes, with 24 bytes being recommended
   (as it allows for an ACPI field at the end).

   The layout of a memory map entry (using this function) is the same as mmapEntry{}.

*/

uint32 GetMmapEntry(void* Buffer, uint32 Size, uint32 Continuation) {

  // Prepare to go into real mode in order to call our BIOS function (int 15h / eax E820h).

  // We'll need eax to be E820h, ebx to be the continuation number, ecx to be the size,
  // edx to be a special signature ('SMAP' or 534D4150h), and es:di to be our buffer.

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = 0xE820;
  Table->Ebx = Continuation;
  Table->Ecx = Size;
  Table->Edx = 0x534D4150;

  Table->Es = (uint16)((int)(Buffer) >> 4);
  Table->Di = (uint16)((int)(Buffer) & 0x0F);

  // We can now call our BIOS function. We're in protected mode, so we'll need to temporarily
  // return to real mode to actually do so.

  Table->Int = 0x15;
  RealMode();

  // Return the continuation number, or just 0 if the carry flag is set. In this case, the
  // carry flag being set has the same meaning as a continuation number of 0.

  if (hasFlag(Table->Eflags, CarryFlag)) {

    return 0;

  }

  return Table->Ebx;

}



/* ... */

// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS
// TODO - THIS NEEDS COMMENTS

uint64 AllocFromUsableMmap(uint64 Address, uint32 Size, mmapEntry* UsableMmap, uint8 NumUsableMmapEntries) {

  // (First, let's find the mmap entry that corresponds to Address, or at the very
  // least, the closest one)

  uint8 Entry = (NumUsableMmapEntries - 1);

  while (Address < UsableMmap[Entry].Base) {

    if (Entry == 0) {

      Address = UsableMmap[0].Base;
      break;

    }

    Entry--;

  }

  // (Okay - now, let's see if there's enough space in this entry already, and if not,
  // go to the next entry and/or return 0).

  while (Entry < NumUsableMmapEntries) {

    // (Is there enough space?)

    if ((Address + Size) < GetEndOfMmapEntry(UsableMmap[Entry])) {
      return (Address + Size);
    } else {
      Entry++;
    }

    // (If there isn't, then..)

    if (Entry >= NumUsableMmapEntries) {
      return 0;
    } else {
      Address = UsableMmap[Entry].Base;
    }

  }

  // (Finally, if we've gotten here, then just return 0, since there's no
  // space left)

  return 0;

}
