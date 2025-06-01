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

  void* Pointer;

  struct _allocationNode* Previous;
  struct _allocationNode* Next;

} allocationNode;

static constexpr uint64 ScalingFactor = (SystemPageSize / sizeof(allocationNode));



// (TODO - Variables and such)

allocationNode* Nodes[64] = {NULL};
uint8 Levels[2] = {0};

static usableMmapEntry* KernelMmap = NULL;
static uint16 NumKernelMmapEntries = 0;



// (TODO - Push node to LatestNodes[Level], via Address (which represents
// the memory region this is supposed to represent))

static inline void PushNode(uintptr Address, void* Pointer, uint16 Level) {

  // (Check if `Address` or `Pointer` are null, and if they are, return;
  // otherwise, declare a node at the given address.)

  if (Address == 0) {
    return;
  } else if (Pointer == NULL) {
    return;
  }

  allocationNode* Node = (allocationNode*)Address;

  // (Get the previous node to point to this one, if possible)

  if (Nodes[Level] != NULL) {
    Nodes[Level]->Next = Node;
  }

  // (Add the previous node, set the next node to NULL (as this is the
  // last node), and update Nodes[Level] to point to this node)

  Node->Pointer = Pointer;
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

  // Remove the `Next` pointer from that node, if applicable.

  if (Nodes[Level] != NULL) {
    Nodes[Level]->Next = NULL;
  }

  // Return.

  return;

}



// (TODO - Initializer/constructor function)

bool InitializeAllocationSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries) {

  // (First, is this actually a usable memory map?)

  if (UsableMmap == NULL) {
    return false;
  } else if (NumUsableMmapEntries == 0) {
    return false;
  }

  KernelMmap = UsableMmap;
  NumKernelMmapEntries = NumUsableMmapEntries;

  // (Calculate the 'minimum' and 'maximum' levels, storing them in
  // Levels[]; this is based off of `SystemPageSize`

  auto PageSize = SystemPageSize;
  Levels[0] = 0; Levels[1] = 63;

  while (PageSize != 0) {

    Levels[0]++;
    PageSize >>= 1;

  }

  Levels[0]--;

  // (Handle each memory map entry)

  for (auto Entry = 0; Entry < NumKernelMmapEntries; Entry++) {

    // First, let's calculate how much space will actually be needed for
    // the allocation data (at *Start), and reserve it.

    auto ReservedSpace = (KernelMmap[Entry].Limit / ScalingFactor);

    if ((ReservedSpace % SystemPageSize) != 0) {
      ReservedSpace += (SystemPageSize - (ReservedSpace % SystemPageSize));
    }

    uintptr Start = KernelMmap[Entry].Base;
    uintptr Offset = 0;

    extern void Printf(const char* String, bool Important, uint8 Color, ...);
    Printf("(Reserved - [%xh,%xh]) (Free - [%xh,%xh])\n\r", false, 0x0B, Start, (Start+ReservedSpace), (Start+ReservedSpace), (Start+KernelMmap[Entry].Limit));

    // Now that we know how much space we're working with, let's
    // push nodes (add free blocks) as necessary.

    auto Space = (KernelMmap[Entry].Limit - ReservedSpace);

    if (Space < (KernelMmap[Entry].Limit / 2)) {
      return false;
    }

    while (Space >= SystemPageSize) {

      // (Find the largest offset of two that would reasonably fit;
      // this is the base-two logarithm of `Space`)

      auto Logarithm = 0;

      while ((Space >> Logarithm) != 0) {
        Logarithm++;
      }

      Logarithm--;

      // (Push a node to signify that there's a free memory area
      // at *(Start + Offset) that's (1 << Logarithm) bytes wide)

      uintptr Address = (Start + (Offset / ScalingFactor));
      void* Pointer = (void*)(Start + ReservedSpace + Offset);

      PushNode(Address, Pointer, Logarithm);

      // (Update `Offset` and `Space` respectively)

      Offset += (1ULL << Logarithm);
      Space -= (1ULL << Logarithm);

    }

  }

  // (Return true, now that we're done)

  return true;

}



// (TODO - Kernel memory allocator, malloc(); returns `NULL` if there's
// an issue (size == 0, lack of memory, etc.), and must be contiguous)

void* Malloc(uint64 Size) {

  // (Sanity-check our values first - if `Size` is zero or `KernelMmap`
  // is a null pointer (or has no entries), return NULL)

  if ((KernelMmap == NULL) || (NumKernelMmapEntries == 0)) {
    return NULL;
  } else if (Size == 0) {
    return NULL;
  }

  // Now that we know we're probably good to go, let's calculate the
  // (minimum) block size we need to allocate.

  auto BlockLevel = Levels[0];
  auto BlockSize = SystemPageSize;

  while (Size > SystemPageSize) {

    BlockLevel++;
    BlockSize *= 2;

    Size /= 2;

  }

  // (If the block level exceeds the maximum limit, just return NULL)

  if (BlockLevel > Levels[1]) {
    return NULL;
  }

  extern void Printf(const char* String, bool Important, uint8 Color, ...);
  Printf("[Malloc] BlockLevel = %d, Levels[n] = (%d,%d)\n\r", false, 0x0F, (uint64)BlockLevel, (uint64)Levels[0], (uint64)Levels[1]);

  // The allocation system we're using uses doubly-linked lists of nodes,
  // with one list for each 'level' (which corresponds to the amount
  // of memory that node corresponds to).

  // (Essentially, for any N, Nodes[N] is a pointer to the last entry in
  // a list of nodes that correspond to memory areas that are (1 << N)
  // bytes long.)

  // That means there are two steps we need to take:

  // -> (1) If there *aren't* any memory areas at our level, try to
  // find a larger area we can break up)

  if (Nodes[BlockLevel] == NULL) {

    for (auto Level = (BlockLevel + 1); Level <= Levels[1]; Level++) {

      // [TODO] Break up blocks

    }

  }

  // -> (2) Otherwise, or after finishing that, obtain the memory area
  // from Nodes[BlockLevel], popping it from the list, and returning.

  void* Pointer = NULL;

  if (Nodes[BlockLevel] != NULL) {

    Pointer = Nodes[BlockLevel]->Pointer;
    Printf("[Malloc] Found something! (@ %xh, MPointer -> %xh, Previous -> %xh)\n\r", false, 0x0F, (uint64)Nodes[BlockLevel], (uint64)Nodes[BlockLevel]->Pointer, (uint64)Nodes[BlockLevel]->Previous);

    if (Pointer != NULL) {
      PopNode(BlockLevel);
    }

  }

  // (Return `Pointer`; if we didn't find anything, this will return
  // a null pointer, which is also appropriate)

  return Pointer;

}



// (TODO - Memory deallocator, free(); does nothing if `Pointer == NULL`)

void Free(void* Pointer) {

  // (TODO)
  return;

}
