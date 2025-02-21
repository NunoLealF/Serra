// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Shared/Stdint.h"
#include "../Memory.h"

// TODO - paging-related functions.

// (This is basically just a simple implementation, but in the future, try
// to make specific #defines for flags, etc. etc.; also, the address doesn't
// need to go through anything too weird for the most part



// TODO/DEBUG: (4kib page)

uint64 MakePteEntry(uint64 Address, bool Xd, bool Global, bool CacheDisable, bool WriteThrough, bool User, bool Rw) {

  // Initialize the entry itself (the present (P) bit is always set to 1).

  uint64 Entry = (1 << 0);

  // Write the necessary flag bits to the entry; we ignore the accessed (A),
  // dirty (D), PAT and Protection Key bits, setting them to zero.

  if (Rw == true) (Entry |= (1 << 1)); // RW bit
  if (User == true) (Entry |= (1 << 2)); // User/supervisor bit
  if (WriteThrough == true) (Entry |= (1 << 3)); // PWT bit
  if (CacheDisable == true) (Entry |= (1 << 4)); // PCT bit
  if (Global == true) (Entry |= (1 << 8)); // Global bit
  if (Xd == true) (Entry |= (1ULL << 63)); // XD bit.

  // Write the page frame address to the entry (from bit 12), and return.

  Entry |= (Address & (~0ULL << 12)); // Physical address of the 4KiB page frame

  return Entry;

}



// TODO/DEBUG: (2mib page)

uint64 MakePdeEntry(uint64 Address, bool Xd, bool Global, bool CacheDisable, bool WriteThrough, bool User, bool Rw) {

  // Initialize the entry itself (the present (P) bit is always set to 1).
  // We also set the PS (page size) flag to make this a 2MiB page.

  uint64 Entry = (1 << 0) | (1 << 7);

  // Write the necessary flag bits to the entry; we ignore the accessed (A),
  // dirty (D), PAT and Protection Key bits, setting them to zero.

  if (Rw == true) (Entry |= (1 << 1)); // RW bit
  if (User == true) (Entry |= (1 << 2)); // User/supervisor bit
  if (WriteThrough == true) (Entry |= (1 << 3)); // PWT bit
  if (CacheDisable == true) (Entry |= (1 << 4)); // PCT bit
  if (Global == true) (Entry |= (1 << 8)); // Global bit
  if (Xd == true) (Entry |= (1ULL << 63)); // XD bit.

  // Write the page frame address to the entry (from bit 21), and return.

  Entry |= (Address & (~0ULL << 21)); // Physical address of the 2MiB page frame.

  return Entry;

}



// TODO/DEBUG: (Works equally for PML2/PML3/PML4 (PDE, PDPTE, PML4e))

uint64 MakePmlEntry(uint64 Address, bool Xd, bool CacheDisable, bool WriteThrough, bool User, bool Rw) {

  // Initialize the entry itself (the present (P) bit is always set to 1).

  uint64 Entry = (1 << 0);

  // Write the necessary flag bits to the entry; we ignore the accessed (A),
  // PAT and Protection Key bits, setting them to zero.

  if (Rw == true) (Entry |= (1 << 1)); // RW bit
  if (User == true) (Entry |= (1 << 2)); // User/supervisor bit
  if (WriteThrough == true) (Entry |= (1 << 3)); // PWT bit
  if (CacheDisable == true) (Entry |= (1 << 4)); // PCT bit
  if (Xd == true) (Entry |= (1ULL << 63)); // XD bit.

  // Write the page table address to the entry (from bit 12), and return.

  Entry |= (Address & ~(0ULL << 12)); // Physical address of the page table

  return Entry;

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
