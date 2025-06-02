// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Libraries/String.h"
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



// (TODO - Variables and such)

allocationNode* Nodes[64] = {NULL};
uint8 Limits[2] = {0};

static usableMmapEntry* KernelMmap = NULL;
static uint16 NumKernelMmapEntries = 0;

static constexpr uint64 ScalingFactor = (SystemPageSize / sizeof(allocationNode));



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
  // Limits[]; this is based off of `SystemPageSize`

  auto PageSize = SystemPageSize;
  Limits[0] = 0; Limits[1] = 63;

  while (PageSize != 0) {

    Limits[0]++;
    PageSize >>= 1;

  }

  Limits[0]--;

  // (Handle each memory map entry)

  allocationNode* PreviousNode = NULL;

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

    // We don't know if the reserved space itself is clear or not, so
    // let's clear it with Memset() - this is useful later on.

    Memset((void*)KernelMmap[Entry].Base, 0, ReservedSpace);

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

      allocationNode* Node = (allocationNode*)Address;

      Node->Pointer = Pointer;
      Node->Position.Previous = PreviousNode;
      Node->Position.Next = NULL;

      if (PreviousNode != NULL) {
        PreviousNode->Position.Next = Node;
      }

      PreviousNode = Node;

      Node->Size.Previous = Nodes[Logarithm];
      Nodes[Logarithm] = Node;

      // (Update `Space` and `Offset` respectively)

      Space -= (1ULL << Logarithm);
      Offset += (1ULL << Logarithm);

    }

  }

  // (Return true, now that we're done)

  return true;

}



// (TODO - Push a block (for FreeBlock, Merge, etc.))

static inline void PushBlock(allocationNode* Node, void* Pointer, allocationNode* Previous, allocationNode* Next, uint8 Level) {

  // (Create a node at the address pointed to by `Node`, and link it
  // with adjacent nodes if possible)

  Node->Pointer = Pointer;

  Node->Position.Previous = Previous;
  Node->Position.Next = Next;

  if (Previous != NULL) {
    Previous->Position.Next = Node;
  }

  if (Next != NULL) {
    Next->Position.Previous = Node;
  }

  // (Push the node to its corresponding list at `Nodes[Level]`,
  // and link it with the last free size block)

  Node->Size.Previous = Nodes[Level];
  Nodes[Level] = Node;

  extern void Printf(const char* String, bool Important, uint8 Color, ...);
  Printf("[PushBlock] Found something! (Node -> %xh, Pointer -> %xh, Level -> %d, Size -> %xh)\n\r", false, 0x0D,
          (uint64)Node, (uint64)Pointer, (uint64)Level, (uint64)(1ULL << Level));

  // (Return, now that we're done)

  return;

}



// (TODO - Pop a block (for Allocate(), Merge, etc.))

[[nodiscard]] static inline void* PopBlock(uint8 Level) {

  // (Get the current node for the corresponding level, as long as
  // it isn't a null pointer)

  allocationNode* Node = Nodes[Level];
  void* Pointer = NULL;

  if (Node != NULL) {

    // (Update `Pointer`, and check that it isn't NULL either)

    Pointer = Node->Pointer;

    extern void Printf(const char* String, bool Important, uint8 Color, ...);
    Printf("[PopBlock] Found something! (@ %xh, Pointer -> %xh, prevS -> %xh, prevP/nextP = %x/%xh)\n\r", false, 0x0C,
            (uint64)Node, (uint64)Node->Pointer,
            (uint64)Node->Size.Previous, (uint64)Node->Position.Previous,
            (uint64)Node->Position.Next);

    if (Pointer != NULL) {

      // (Relink any adjacent nodes, and update position information)

      allocationNode* Previous = Node->Position.Previous;
      allocationNode* Next = Node->Position.Next;

      if (Previous != NULL) {
        Previous->Position.Next = Next;
      }

      if (Next != NULL) {
        Next->Position.Previous = Previous;
      }

      // (Pop the node from `Nodes[Level]`)

      allocationNode* Node = Nodes[Level];
      Nodes[Level] = Node->Size.Previous;

      // (Clear our newly popped node, so it doesn't appear in any
      // searches from something like `FreeBlock()`)

      Memset((void*)Node, 0, sizeof(allocationNode));

    }

  }

  // (Now that we're done, return `Pointer`)

  return Pointer;

}



// (TODO - Free a block (for Free()))

static inline allocationNode* FreeBlock(void* Pointer, uintptr Size) {

  // First, let's try to figure out which entry `Pointer` belongs to.

  // (Try to figure out where `Pointer` is (within the memory map))

  auto Entry = uintmax;

  for (uint16 Index = 0; Index < NumKernelMmapEntries; Index++) {

    if ((uintptr)Pointer >= KernelMmap[Index].Base) {

      if ((uintptr)Pointer <= (KernelMmap[Index].Base + KernelMmap[Index].Limit)) {

        Entry = Index;
        break;

      }

    }

  }

  // (If it's not within the memory map at all, just return false)

  if (Entry == uintmax) {
    return NULL;
  }

  // Next, let's calculate the amount of reserved space within this
  // entry, and return NULL if `Pointer` is within it.see

  // (Calculate the amount of reserved space in the entry that
  // `Pointer` belongs to)

  auto ReservedSpace = (KernelMmap[Entry].Limit / ScalingFactor);

  if ((ReservedSpace % SystemPageSize) != 0) {
    ReservedSpace += (SystemPageSize - (ReservedSpace % SystemPageSize));
  }

  // (Check if `Pointer` is within that reserved space, and if so,
  // return a NULL pointer)

  auto Start = KernelMmap[Entry].Base;

  if ((uintptr)Pointer <= (Start + ReservedSpace)) {
    return NULL;
  }

  // Now that we know how much reserved space we have, we can move
  // onto trying to find adjacent free memory blocks.

  // Each free memory block is stored in a corresponding unique location
  // as an `allocationNode`, which means you can predictably find
  // adjacent nodes like this:

  // (1) Calculate the corresponding address for the node that
  // would represent `Pointer`

  auto Offset = ((uintptr)Pointer - (ReservedSpace + Start));
  uintptr Address = (Start + (Offset / ScalingFactor));

  // (2) Try to find the *previous* node
  // (If the `Pointer` member isn't NULL, that means it's a valid node)

  allocationNode* PreviousNode = NULL;

  auto Limit = (KernelMmap[Entry].Base);
  auto Multiplier = sizeof(allocationNode);

  while ((Address - Multiplier) >= Limit) {

    allocationNode* Try = (allocationNode*)(Address - Multiplier);

    if (Try->Pointer != NULL) {
      PreviousNode = Try; break;
    } else {
      Multiplier *= 2;
    }

  }

  // (3) Try to find the *next* node.
  // (If the `Pointer` member isn't NULL, that means it's a valid node)

  allocationNode* NextNode = NULL;

  Limit += ReservedSpace;
  Multiplier = sizeof(allocationNode);

  while ((Address + Multiplier) <= Limit) {

    allocationNode* Try = (allocationNode*)(Address + Multiplier);

    if (Try->Pointer != NULL) {
      NextNode = Try; break;
    } else {
      Multiplier *= 2;
    }

  }

  extern void Printf(const char* String, bool Important, uint8 Color, ...);
  Printf("[FreeBlock] On @%xh (guess=%xh), guessing that (PreviousNode -> %xh), (NextNode -> %xh)\n\r", false, 0x0B,
          (uint64)Address, (uint64)Pointer, (uint64)PreviousNode, (uint64)NextNode);

  // Finally, now that we've found the previous and next nodes, we
  // can create a new node (at *Address), and assign it to the given
  // list, depending on the size:

  // (Calculate the logarithm of the block size / the block 'level')

  auto Logarithm = Limits[0];

  while (Size > (1ULL << Logarithm)) {
    Logarithm++;
  }

  // (Return a null pointer if the logarithm exceeds the maximum)

  if (Logarithm > Limits[1]) {
    return NULL;
  }

  // (Use PushBlock() to initialize the node at the corresponding
  // location (Address), before returning it)

  allocationNode* Node = (allocationNode*)Address;
  PushBlock(Node, Pointer, PreviousNode, NextNode, Logarithm);

  return Node;

}



// (TODO - Break a block up to a certain target size)

[[nodiscard]] static inline bool DivideBlock(allocationNode* Node, uint8 Level, uint8 Target) {

  // (Sanity-check `Node`, `Level` and `Target`, and return `false`
  // if they don't make sense)

  if ((Node == NULL) || (Node->Pointer == NULL)) {
    return false;
  } else if (Target >= Level) {
    return false;
  }

  uintptr Address = (uintptr)Node;

  // (Additionally, check if we're exceeding Limits[])

  if (Target < Limits[0]) {
    return false;
  } else if (Level > Limits[1]) {
    return false;
  }

  // We need to be able to divide our memory block in the most efficient
  // way possible, depending on the value of `Target`; so, before we
  // do anything else, let's try to deal with that.

  // (Calculate the necessary size for the 'buffer')

  auto Size = (Level - Target + 1);
  uint8 Buffer[Size];

  // (Fill it out; for example, (15, 11) becomes (14, 13, 12, 11, 11))

  extern void Printf(const char* String, bool Important, uint8 Color, ...);
  Printf("[DivideBlock] (%d,%d) turned into (", false, 0x0A, (uint64)Level, (uint64)Target);

  for (auto Index = 0; Index < (Size - 1); Index++) {
    Buffer[Index] = (Level - Index - 1);
    Printf("%d, ", false, 0x0F, (uint64)Buffer[Index]);
  }

  Buffer[Size - 1] = Target;
  Printf("%d)\n\r", false, 0x0F, (uint64)Target);

  // Now that we know which levels we need to push to, let's extract the
  // current node's information, before popping it from its list.

  allocationNode* PreviousNode = Node->Position.Previous;
  allocationNode* NextNode = Node->Position.Next;

  void* Pointer = PopBlock(Level);

  // (Sanity-check `Pointer`, just in case it didn't go well)

  if (Pointer == NULL) {
    return false;
  }

  // Now, let's add *new* free blocks to their respective lists, as
  // indicated by the buffer.

  uintptr Location = (uintptr)Pointer;

  for (auto Index = 0; Index < Size; Index++) {

    // (Declare a node, and push it with `PushBlock()`

    allocationNode* Node = (allocationNode*)Address;
    PushBlock(Node, (void*)Location, PreviousNode, NextNode, Buffer[Index]);

    // (Update `PreviousNode` to point to this node - `NextNode`
    // shouldn't need to be updated, however)

    PreviousNode = Node;

    // (Update `Address` and `Location` to point to the next block)

    Address += ((1ULL << Buffer[Index]) / ScalingFactor);
    Location += (1ULL << Buffer[Index]);

  }

  // (Now that we're done, we can safely return `true`.)

  return true;

}



// (TODO - Join/merge a block; can be recursive)

[[nodiscard]] static inline bool MergeBlock(allocationNode* Node, uintptr Size) {

  // (Sanity-check `Node` and `Size`, and return `false` if they
  // don't make sense)

  if (Node == NULL) {
    return false;
  } else if (Size == 0) {
    return false;
  }

  // Before we do anything else, let's calculate the level (logarithm) of
  // our node, based on its size.

  auto Logarithm = Limits[0];

  while (Size > (1ULL << Logarithm)) {
    Logarithm++;
  }

  // Next, let's recursively merge the previous block with this one
  // until there are no more blocks left to merge, or until
  // `Logarithm` goes out of bounds).

  while (Logarithm < Limits[1]) {

    // Calculate the address of the previous node (and break if it
    // appears to be invalid.)

    allocationNode* Previous = Node->Position.Previous;
    void* Pointer = Previous->Pointer;

    if (Previous == NULL) {
      break;
    } else if ((uintptr)Node < (uintptr)Previous) {
      break;
    }

    // Compare the distance (delta) in addresses, and break if it
    // isn't equal to `(1 << Logarithm)`.

    uintptr Delta = ((uintptr)Node - (uintptr)Previous);

    if (Delta != ((1ULL << Logarithm) / ScalingFactor)) {
      break;
    }

    extern void Printf(const char* String, bool Important, uint8 Color, ...);
    Printf("[MergeBlock] Nodes at (%xh, %xh) are likely one %xh-byte entry \n\r", false, 0x09,
            (uint64)Previous, (uint64)Node, (Delta * ScalingFactor * 2));

    // Finally, now that we know that we *can* merge these two
    // blocks, let's go ahead and do so. We'll need to:

    // -> (1) 'Pop' the last two nodes from their respective lists;
    // we can't use PopBlock() since we don't know where they are.

    allocationNode* Before = Previous->Position.Previous;
    allocationNode* After = Node->Position.Next;

    Printf("[MergeBlock] Popping from stack, so (%xh --(%xh,%xh)--> %xh)\n\r", false, 0x09,
            (uint64)Before, (uint64)Previous, (uint64)Node, (uint64)After);

    if (Before != NULL) {
      Before->Position.Next = After;
    }

    if (After != NULL) {
      After->Position.Previous = Before;
      After->Size.Previous = Before;
    }

    if (Nodes[Logarithm] == Node) {
      Nodes[Logarithm] = Before;
    }

    Memset((void*)Previous, 0, sizeof(allocationNode));
    Memset((void*)Node, 0, sizeof(allocationNode));

    // -> (2) 'Push' a new node to the level above, although this
    // time using PushBlock().

    Logarithm++;
    PushBlock(Previous, Pointer, Before, After, Logarithm);

    Printf("[MergeBlock] Pushing to stack, making new %xh-byte entry at %xh \n\r", false, 0x09,
            (uint64)(1ULL << Logarithm), (uint64)Previous);

    // Finally, let's redefine `Node` to point to our previous
    // block, and repeat the loop.

    Node = Previous;
    break;

  }

  // (Now that we're done, we can safely return `true`.)

  return true;

}



// (TODO - Kernel memory allocator, malloc(); returns `NULL` if there's
// an issue (size == 0, lack of memory, etc.), and must be contiguous)

[[nodiscard]] void* Malloc(const uintptr* Length) {

  // (Sanity-check our values first - if `Length` is zero or `KernelMmap`
  // is a null pointer (or has no entries), return NULL)

  if ((KernelMmap == NULL) || (NumKernelMmapEntries == 0)) {
    return NULL;
  } else if (Length == NULL) {
    return NULL;
  } else if (*Length == 0) {
    return NULL;
  }

  uintptr Size = *Length;

  // Now that we know we're probably good to go, let's calculate the
  // (minimum) block size we need to allocate.

  auto BlockLevel = Limits[0];

  while (Size > (1ULL << BlockLevel)) {
    BlockLevel++;
  }

  // (If the block level exceeds the maximum limit, just return NULL)

  if (BlockLevel > Limits[1]) {
    return NULL;
  }

  extern void Printf(const char* String, bool Important, uint8 Color, ...);
  Printf("[Malloc] BlockLevel = %d, Limits[n] = (%d,%d)\n\r", false, 0x0E, (uint64)BlockLevel, (uint64)Limits[0], (uint64)Limits[1]);

  // The allocation system we're using uses doubly-linked lists of nodes,
  // with one list for each 'level' (which corresponds to the amount
  // of memory that node corresponds to).

  // (Essentially, for any N, Nodes[N] is a pointer to the last entry in
  // a list of nodes that correspond to memory areas that are (1 << N)
  // bytes long.)

  // That means there are two steps we need to take:

  // -> (1) If there *aren't* any memory areas at our level, try to
  // find a larger area we can divide)

  if (Nodes[BlockLevel] == NULL) {

    for (auto Level = (BlockLevel + 1); Level <= Limits[1]; Level++) {

      if (Nodes[Level] != NULL) {

        bool Result = DivideBlock(Nodes[Level], Level, BlockLevel);

        if (Result == false) {
          return NULL;
        } else {
          break;
        }

      }

    }

  }

  // -> (2) TODO - Use PopBlock() to actually pop it from the stack;
  // it should return the corresponding address as well

  return PopBlock(BlockLevel);

}



// (TODO - Memory deallocator, free(); does nothing if `Pointer == NULL`)

[[nodiscard]] bool Free(void* Pointer, const uintptr* Length) {

  // (Sanity-check our values first - if `Pointer` *or* `KernelMmap`
  // is a null pointer (or has no entries), return false)

  if ((KernelMmap == NULL) || (NumKernelMmapEntries == 0)) {
    return false;
  } else if ((Pointer == NULL) || (Length == NULL)) {
    return false;
  } else if (*Length == 0) {
    return false;
  }

  uintptr Size = *Length;

  // -> (1) TODO - Use FreeBlock() to actually free the block, and
  // have it return the corresponding node.

  allocationNode* Node = FreeBlock(Pointer, Size);

  if (Node == NULL) {
    return false;
  }

  // -> (2) If possible, merge Nodes[N] with any other nearby
  // blocks (for example, two 4 KiB blocks -> one 8 KiB block).

  return MergeBlock(Node, Size);

}
