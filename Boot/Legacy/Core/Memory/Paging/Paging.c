// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Shared/Stdint.h"
#include "../Memory.h"

// TODO - paging-related functions.

// (In hindsight, this needs to be)




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

   For example, if you wanted to allocate two memory blocks, that are ABCh
   and DEFh bytes long respectively, you could do the following:

   -> uint64 Start = UsableMmap[0].Base;
   -> uint64 MemoryBlockA = AllocateFromMmap(Start, 0xABC, UsableMmap, NumEntries);
   -> uint64 MemoryBlockB = AllocateFromMmap(MemoryBlockA, 0xDEF, UsableMmap, NumEntries);

*/

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

    // (Is there enough space?)

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

  return 0;

}
