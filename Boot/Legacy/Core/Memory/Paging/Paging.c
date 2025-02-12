// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Shared/Stdint.h"
#include "../Memory.h"

// TODO - paging-related functions.

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
