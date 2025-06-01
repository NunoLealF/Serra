// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../System/System.h"
#include "../../Common.h"
#include "Memory.h"

// (TODO - Include a function to initialize the memory allocator)

// I'm going to need to do that for *each* memory region, and have metadata
// at the very start, with statistics like

// -> how many free (4KiB, 8KiB, ..., (whatever the limit is)) pages?
// -> when does this actually start and end?
// -> pointers to all the different bitmaps, ofc.

// If I have a bitmap for each level, and an infinite amount of levels (in
// this case, 52, up to the 64-bit limit - just for efficiency), then in
// theory the bitmaps together should occupy (N/16384) bytes, or (N/8192)
// if it's 2 bits each, etc.

// A somewhat clever solution would be.. since this is a buddy allocator,
// let those 2 bits define whether the 1st and 2nd are occupied or not,
// so 00 is free, 01 or 10 is partial (either left or right are not),
// 11 is..


// I could also use a stack in a similar way - make one for 4KB, one for
// 8 KB, etc.; then, you can split it into blocks and such.

// (Although this is a little problematic because you need potentially
// up to 8 bytes for each page => 1/512.. i mean, it wouldn't be the end
// of the world, but idk.)

// Is that what a slab allocator is? It might be, actually; it combines
// the benefits of a buddy allocator with the.. hmm

// There's an issue though, which is that.. because it's a *stack*, it..
// hmm, actually, I could just solve that with a linked list, I think.


// Right, so, using a linked-list-style stack, you could substantially
// reduce the amount of space you'd have to *reserve*; at the cost of
// still needing some way of dynamically allocating memory.

// But you can fix that by *always* allocating stacks for large areas,
// and then only breaking them up once needed, *and* dynamically
// allocating more when needed from the same space - that way
// the initialization cost is basically 0

// It would be more complicated, and I'd probably have a headache
// trying to implement it, but it seems good, honestly - it also really
// minimizes memory fragmentation

// That being said.. now that I think about it, it only minimizes
// fragmentation on allocating, but on *freeing*, it's not going
// to be easy to actually build those blocks back together.

// You're breaking it into individual pieces, but then not offering
// anything that would tell you how to put it back together, so unless
// you manually solve the puzzle (hard, don't do this).. I don't know

// One solution would be to basically sort it.. and, maybe? A linked
// list also solves this, you just have to add a new object/point, no
// need to resort the whole list.

// That being said, linked lists are O(1) to add new objects to, but
// O(n) to traverse. You could automatically add adjacent blocks, and
// even push them at the top of the stack, but to *know* where to
// add the newly enbiggened node would be a hassle

// Linked lists are also slow (if they aren't consecutive) because
// of prefetching / non-contiguous memory access

// Wait, right, I get it!! It's a linked list, *traversing* is slow
// but if you sort it.. or, well, hmm


// I mean, I *can* organize it in memory, in such a way that it's
// actually fast (just make it so that each node is stored at an index
// dependent on the address, so you can 'scan' the memory for nodes)

// But that's complicated, and I frankly don't have much time to
// finish this at all - I need this done by *today*, c'mon)



// (TODO - Node; in this case, `Previous` and `Next` refer to previous
// and next nodes *with the same size*, and can be NULL if undefined)

typedef struct _allocationNode {

  struct _allocationNode* Previous;
  struct _allocationNode* Next;

} allocationNode;



// (TODO - Variables and such)

allocationNode* Nodes[64] = {NULL};
uint8 Levels[2] = {0};



// (TODO - Push node to LatestNodes[Level], via Address (which represents
// the memory region this is supposed to represent))

static inline void PushNode(uintptr Address, uint16 Level) {

  // Check if `Address` is zero, and if it isn't, declare a node
  // at the given address.

  if (Address == 0) {
    return;
  }

  allocationNode* Node = (allocationNode*)Address;

  // Get the previous node to point to this one, if possible.

  if (Nodes[Level] != NULL) {
    Nodes[Level]->Next = Node;
  }

  // Add the previous node, set the next node to NULL (as this is the
  // last node), and update Nodes[Level] to point to this node.

  Node->Previous = Nodes[Level];
  Node->Next = NULL;

  Nodes[Level] = Node;

  // Return.

  return;

}



// (TODO - Pop node from LatestNodes[Level])

static inline void PopNode(uint16 Level) {

  // Check if Nodes[Level] is NULL, and if not, set the last node to
  // the previous node.

  if (Nodes[Level] == NULL) {
    return;
  }

  Nodes[Level] = Nodes[Level]->Previous;

  // Remove the `Next` pointer from that node.

  Nodes[Level]->Next = NULL;

  // Return.

  return;

}



// (TODO - Initializer/constructor function)

bool InitializeAllocationSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries) {

  // (Is this actually a usable memory map?)

  if (UsableMmap == NULL) {
    return false;
  } else if (NumUsableMmapEntries == 0) {
    return false;
  }

  usableMmapEntry* KernelMmap = UsableMmap;

  // (Calculate the 'minimum' and 'maximum' levels, storing them in
  // Levels[]; this is based off of `SystemPageSize`

  auto PageSize = SystemPageSize;
  Levels[0] = 0; Levels[1] = 63;

  while (PageSize != 0) {

    Levels[0]++;
    PageSize >>= 1;

  }

  // (Handle each memory map entry)

  for (auto Entry = 0; Entry < NumUsableMmapEntries; Entry++) {

    // First, let's calculate how much space will actually be needed for
    // the allocation data (at *Start), and reserve it.

    auto ReservedSpace = (KernelMmap[Entry].Limit * sizeof(allocationNode)) / SystemPageSize;

    if ((ReservedSpace % SystemPageSize) != 0) {
      ReservedSpace += (SystemPageSize - (ReservedSpace % SystemPageSize));
    }

    uintptr Start = KernelMmap[Entry].Base;
    uintptr Offset = Start;

    extern void Printf(const char* String, bool Important, uint8 Color, ...);
    Printf("(Reserved - [%xh,%xh]) (Free - [%xh,%xh])\n\r", false, 0x0B, Start, (Start+ReservedSpace), (Start+ReservedSpace), (Start+KernelMmap[Entry].Limit));

    // Now that we know how much space we're working with, let's
    // push nodes (add free blocks) as necessary.

    auto Space = (KernelMmap[Entry].Limit - ReservedSpace);

    if (Space < (KernelMmap[Entry].Limit / 2)) {
      return false;
    }

    while (Space > SystemPageSize) {

      // (Find the largest offset of two that would reasonably fit;
      // this is the base-two logarithm of `Space`)

      auto Logarithm = 0;

      while ((Space >> Logarithm) != 0) {
        Logarithm++;
      }

      Logarithm--;

      // (Push a node to signify that there's a free memory area
      // at *Offset that's (1 << Logarithm) bytes wide)

      extern void Printf(const char* String, bool Important, uint8 Color, ...);
      Printf("(Space = %xh, Offset = %xh~%xh, Logarithm = %d, (1 << Logarithm) = %xh)\n\r", false, 0x0F, (uint64)Space, (uint64)Offset, (uint64)(Offset + (((1ULL << Logarithm) * sizeof(allocationNode)) / SystemPageSize)), (uint64)Logarithm, (uint64)(1ULL<<Logarithm));

      PushNode(Offset, Logarithm);

      // (Update `Space` and `Offset`)

      Space -= (1ULL << Logarithm);
      Offset += ((1ULL << Logarithm) * sizeof(allocationNode)) / SystemPageSize;

    }

  }

  // (Return true, now that we're done)

  return true;

}


// (TODO - Include a function to allocate (kmalloc? malloc?) and free
// (free? kfree?) memory - maybe like in EFI where they have pool/heap
// and page.)
