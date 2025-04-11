// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Shared/Stdint.h"
#include "../Memory.h"

// TODO - paging-related functions.

// (In hindsight, this needs to be)




// (TODO: Write documentation)
// 4KiB-align offset

static uint64 PageAlign(uint64 Offset) {

  if ((Offset % 0x1000) != 0) {
    Offset += (0x1000 - (Offset % 0x1000));
  }

  return Offset;

}



// TODO: (Other functions, basically)

/* uint64 AllocateFromMmap()

   Inputs: uint64 Start - The address you want to start searching from.

           uint32 Size - The size of the memory block you want to allocate.

           mmapEntry* UsableMmap - An array of memory map entries that only
           correspond to usable memory (the "usable memory map").

           uint8 NumUsableMmapEntries - The number of entries in UsableMmap.

   Outputs: uint64 - If successful, an address that corresponds to the end
            of the newly allocated memory block; *otherwise, zero (0)*.

   The purpose of this function is to allocate a given amount of memory from
   the system's memory map, using a simple bump allocator design.

   Essentially, this function looks for a free memory area in UsableMmap (that
   comes after Start), and either returns an address corresponding to the end
   of our newly allocated memory area, or 0 if it can't find a large enough
   area to be allocated.

   Since it doesn't have a way to free (or unallocate) memory, and isn't the
   most efficient system, this function isn't really comparable to an actual
   memory manager; however, it's *perfectly suitable* for allocating small
   blocks of memory that don't need to be freed, such as page tables.

   Keep in mind that this function doesn't have any notion of what's been
   allocated so far (and what hasn't), so if you're allocating multiple
   things, always make sure to update the starting address with the return
   value from the previous call.

   For example, if you wanted to allocate two memory blocks, that are ABCDh
   and EFABh bytes long respectively, you could do the following:

   -> uint64 Start = UsableMmap[0].Base;
   -> uint64 MemoryBlockA = AllocateFromMmap(Start, 0xABCD, UsableMmap, NumEntries);
   -> uint64 MemoryBlockB = AllocateFromMmap(MemoryBlockA, 0xEFAB, UsableMmap, NumEntries);

*/

void __attribute__((noreturn)) Panic(char* String, uint32 Eip); // So the function can panic when necessary

uint64 AllocateFromMmap(uint64 Start, uint32 Size, mmapEntry* UsableMmap, uint8 NumUsableMmapEntries) {

  // (First, let's find the mmap entry that corresponds to Start, or at the very
  // least, the closest one)

  uint8 Entry = (NumUsableMmapEntries - 1);

  while (Start < UsableMmap[Entry].Base) {

    if (Entry == 0) {

      Start = UsableMmap[0].Base;
      break;

    }

    Entry--;

  }

  // (Okay - now, let's see if there's enough space in this entry already, and if not,
  // go to the next entry and/or return 0)

  while (Entry < NumUsableMmapEntries) {

    // (Align to page boundaries (4KiB), and panic if we exceed the 32-bit limit)

    if ((Start + Size) > 0x100000000) {
      Panic("Attempted to allocate memory beyond 32-bit limit.", 0);
    }

    // (Is there enough space?)

    Start = PageAlign(Start);

    if ((Start + Size) < GetEndOfMmapEntry(UsableMmap[Entry])) {
      return (Start + Size);
    } else {
      Entry++;
    }

    // (If there isn't, then..)

    if (Entry >= NumUsableMmapEntries) {
      return 0;
    } else {
      Start = UsableMmap[Entry].Base;
    }

  }

  // (Finally, if we've gotten here, then just return 0, since there's no
  // space left.)

  Panic("Unable to allocate enough memory.", 0);
  return 0;

}







// (TODO: Write documentation)
// Returns offset. (It's assumed that PhysAddress and VirtAddress are page-aligned)

uint64 InitializePageEntries(uint64 PhysAddress, uint64 VirtAddress, uint64 Size, uint64* Pml4, uint64 Flags, bool UseLargePages, uint64 MmapOffset, mmapEntry* UsableMmap, uint8 NumUsableMmapEntries) {

  // (I don't know how to describe this but we're figuring out the
  // positions within the PMLs of the start/end virt.addresses)

  uint64 Start = PageAlign(VirtAddress);
  uint64 End = (Start + Size);

  uint16 InitialPmls[4] = {(Start >> 39) % 512, (Start >> 30) % 512, (Start >> 21) % 512, (Start >> 12) % 512};
  uint16 FinalPmls[4] = {(End >> 39) % 512, (End >> 30) % 512, (End >> 21) % 512, (End >> 12) % 512};

  // (Now, let's fill out the pages - starting with the PML4s)

  for (uint16 Pml4_Index = InitialPmls[0]; Pml4_Index <= FinalPmls[0]; Pml4_Index++) {

    // Is this filled out already? (PML4 -> array of PML3s)
    // If not, allocate a page for those PML3s.

    uintptr Pml3_Address = (uintptr)(pageAddress(Pml4[Pml4_Index]));

    if (Pml3_Address == 0) {

      MmapOffset = AllocateFromMmap(MmapOffset, 4096, UsableMmap, NumUsableMmapEntries);
      Pml3_Address = (MmapOffset - 4096);

    }

    Pml4[Pml4_Index] = makePageEntry(Pml3_Address, Flags);

    // (Fill out PML3s)

    uint16 Pml3_Start = ((Pml4_Index == InitialPmls[0]) ? InitialPmls[1] : 0);
    uint16 Pml3_End = ((Pml4_Index == FinalPmls[0]) ? FinalPmls[1] : 511);

    uint64* Pml3 = (uint64*)Pml3_Address;

    for (uint16 Pml3_Index = Pml3_Start; Pml3_Index <= Pml3_End; Pml3_Index++) {

      // Is this filled out already? (PML3 -> array of 512 PML2s)
      // If not, allocate a page for those PML2s.

      uintptr Pml2_Address = (uintptr)(pageAddress(Pml3[Pml3_Index]));

      if (Pml2_Address == 0) {

        MmapOffset = AllocateFromMmap(MmapOffset, 4096, UsableMmap, NumUsableMmapEntries);
        Pml2_Address = (MmapOffset - 4096);

      }

      Pml3[Pml3_Index] = makePageEntry(Pml2_Address, Flags);

      // (Fill out PML2s)

      uint16 Pml2_Start = ((Pml4_Index == InitialPmls[0] && Pml3_Index == InitialPmls[1]) ? InitialPmls[2] : 0);
      uint16 Pml2_End = ((Pml4_Index == FinalPmls[0] && Pml3_Index == FinalPmls[1]) ? FinalPmls[2] : 511);

      uint64* Pml2 = (uint64*)Pml2_Address;

      for (uint16 Pml2_Index = Pml2_Start; Pml2_Index <= Pml2_End; Pml2_Index++) {

        // If we're dealing with large pages, we can initialize them now.

        if (UseLargePages == true) {

          Pml2[Pml2_Index] = makePageEntry(PhysAddress, (Flags | pageSize));
          PhysAddress += (4096 * 512);

          continue;

        }

        // Otherwise, repeat the same procedure we did last time

        uintptr Pte_Address = (uintptr)(pageAddress(Pml2[Pml2_Index]));

        if (Pte_Address == 0) {

          MmapOffset = AllocateFromMmap(MmapOffset, 4096, UsableMmap, NumUsableMmapEntries);
          Pte_Address = (MmapOffset - 4096);

        }

        Pml2[Pml2_Index] = makePageEntry(Pte_Address, Flags);

        // (Fill out page table entries)

        uint16 Pte_Start = ((Pml4_Index == InitialPmls[0] && Pml3_Index == InitialPmls[1] && Pml2_Index == InitialPmls[2]) ? InitialPmls[3] : 0);
        uint16 Pte_End = ((Pml4_Index == FinalPmls[0] && Pml3_Index == FinalPmls[1] && Pml2_Index == FinalPmls[2]) ? FinalPmls[3] : 511);

        uint64* Pte = (uint64*)Pte_Address;

        for (uint16 Pte_Index = Pte_Start; Pte_Index <= Pte_End; Pte_Index++) {

          Pte[Pte_Index] = makePageEntry(PhysAddress, Flags);
          PhysAddress += 4096;

        }

      }

    }

  }

  // Return the new offset

  return MmapOffset;


}
