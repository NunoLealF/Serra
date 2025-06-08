// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Libraries/String.h"
#include "../System/System.h"
#include "../../Common.h"
#include "Memory.h"

// (TODO - Global variables)

mmSubsystemData MmSubsystemData = {0};

// (TODO - Local variables)

static usableMmapEntry* KernelMmap = NULL;
static uint16 NumKernelMmapEntries = 0;

static constexpr uint64 ScalingFactor = (SystemPageSize / sizeof(allocationNode));



// (TODO - Initializer/constructor function)

bool InitializeMemoryManagementSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries) {

  // (First, is this actually a usable memory map?)

  if (UsableMmap == NULL) {
    return false;
  } else if (NumUsableMmapEntries == 0) {
    return false;
  }

  KernelMmap = UsableMmap;
  NumKernelMmapEntries = NumUsableMmapEntries;

  // (Calculate the 'minimum' and 'maximum' levels, storing them in
  // MmSubsystemData.Limits[]; this is based off of `SystemPageSize`

  auto PageSize = SystemPageSize;
  MmSubsystemData.Limits[0] = 0; MmSubsystemData.Limits[1] = 63;

  while (PageSize != 0) {

    MmSubsystemData.Limits[0]++;
    PageSize >>= 1;

  }

  MmSubsystemData.Limits[0]--;

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

      Node->Size.Previous = MmSubsystemData.Nodes[Logarithm];
      MmSubsystemData.Nodes[Logarithm] = Node;

      if (Node->Size.Previous != NULL) {
        allocationNode* Temp = Node->Size.Previous;
        Temp->Size.Next = Node;
      }

      // (Update `Space` and `Offset` respectively)

      Space -= (1ULL << Logarithm);
      Offset += (1ULL << Logarithm);

    }

  }

  // (Update `MmSubsystemData.IsEnabled` and return true, now that
  // we're done)

  MmSubsystemData.IsEnabled = true;
  return MmSubsystemData.IsEnabled;

}



// (TODO - Function to test the memory management subsystem)

bool VerifyMemoryManagementSubsystem(const uintptr Try) {

  // (Declare a new `Size` variable with the value from Try, and
  // try to allocate memory with it)

  const uintptr Size = Try;
  void* Pointer = Allocate(&Size);

  if (Pointer == NULL) {
    return false;
  } else if (Size != Try) {
    return false;
  }

  // (Now, try to free the memory block we just allocated, and return
  // the result from Free())

  return Free(Pointer, &Size);

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

  // (Push the node to its corresponding list at `MmSubsystemData.Nodes[Level]`,
  // and link it with the last free size block)

  Node->Size.Previous = MmSubsystemData.Nodes[Level];
  MmSubsystemData.Nodes[Level] = Node;

  if (Node->Size.Previous != NULL) {
    allocationNode* Temp = Node->Size.Previous;
    Temp->Size.Next = Node;
  }

  Node->Size.Next = NULL;

  // (Return, now that we're done)

  return;

}



// (TODO - Pop a block (for Allocate(), Merge, etc.))

[[nodiscard]] static inline void* PopBlock(uint8 Level) {

  // (Get the current node for the corresponding level, as long as
  // it isn't a null pointer)

  allocationNode* Node = MmSubsystemData.Nodes[Level];
  void* Pointer = NULL;

  if (Node != NULL) {

    // (Update `Pointer`, and check that it isn't NULL either)

    Pointer = Node->Pointer;

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

      // (Pop the node from `MmSubsystemData.Nodes[Level]`)

      MmSubsystemData.Nodes[Level] = Node->Size.Previous;

      if (MmSubsystemData.Nodes[Level] != NULL) {
        MmSubsystemData.Nodes[Level]->Size.Next = NULL;
      }

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

  // Finally, now that we've found the previous and next nodes, we
  // can create a new node (at *Address), and assign it to the given
  // list, depending on the size:

  // (Calculate the logarithm of the block size / the block 'level')

  auto Logarithm = MmSubsystemData.Limits[0];

  while (Size > (1ULL << Logarithm)) {
    Logarithm++;
  }

  // (Return a null pointer if the logarithm exceeds the maximum)

  if (Logarithm > MmSubsystemData.Limits[1]) {
    return NULL;
  }

  // (Use PushBlock() to initialize the node at the corresponding
  // location (Node, representing Pointer), before returning it)

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

  // (Additionally, check if we're exceeding MmSubsystemData.Limits[])

  if (Target < MmSubsystemData.Limits[0]) {
    return false;
  } else if (Level > MmSubsystemData.Limits[1]) {
    return false;
  }

  // We need to be able to divide our memory block in the most efficient
  // way possible, depending on the value of `Target`; so, before we
  // do anything else, let's try to deal with that.

  // (Calculate the necessary size for the 'buffer')

  auto Size = (Level - Target + 1);
  uint8 Buffer[Size];

  // (Fill it out; for example, (15, 11) becomes (14, 13, 12, 11, 11))

  for (auto Index = 0; Index < (Size - 1); Index++) {
    Buffer[Index] = (Level - Index - 1);
  }

  Buffer[Size - 1] = Target;

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

  auto Logarithm = MmSubsystemData.Limits[0];

  while (Size > (1ULL << Logarithm)) {
    Logarithm++;
  }

  // Next, let's recursively merge the previous block with this one
  // until there are no more blocks left to merge, or until
  // `Logarithm` goes out of bounds).

  while (Logarithm < MmSubsystemData.Limits[1]) {

    // Calculate the address of the previous and next nodes, and break
    // if both are NULL.

    allocationNode* Previous = Node->Position.Previous;
    allocationNode* Next = Node->Position.Next;

    if ((Previous == NULL) && (Next == NULL)) {
      break;
    }

    // Compare the distance (delta) between nodes, and break if
    // *both* adjacent nodes don't have the same size as ours.

    // (This is necessary because it lets us merge blocks from *both*
    // sides; otherwise, you'd only be able to merge from the right.)

    uintptr Delta = 0;
    auto Target = ((1ULL << Logarithm) / ScalingFactor);

    if (Previous != NULL) {
      Delta = ((uintptr)Node - (uintptr)Previous);
    }

    if (Delta != Target) {

      // (Can we calculate the size of the next block?)

      if (Next == NULL) {

        break;

      } else if (Next->Position.Next == NULL) {

        break;

      } else {

        // (Attempt to calculate the size of the next free block)

        Delta = ((uintptr)Next->Position.Next - (uintptr)Next);

        if (Delta != Target) {

          break;

        } else {

          // (The next free block *is* just as wide, so let's move
          // `Node` to that instead, then continue)

          Previous = Node;
          Node = Next;

        }

      }

    }

    // Finally, now that we know that we *can* merge these two
    // blocks, let's go ahead and do so. We'll need to:

    // -> (1) 'Pop' the last two nodes from their respective lists;
    // we can't use PopBlock() since we don't know where they are.

    // (Deal with position-related links)

    allocationNode* Before = Previous->Position.Previous;
    allocationNode* After = Node->Position.Next;

    void* Pointer = Previous->Pointer;

    if (Before != NULL) {
      Before->Position.Next = After;
    }

    if (After != NULL) {
      After->Position.Previous = Before;
    }

    // (Deal with size-related links, for `Previous`)

    if (Previous->Size.Previous != NULL) {
      allocationNode* Temp = Previous->Size.Previous;
      Temp->Size.Next = Previous->Size.Next;
    } else {
      MmSubsystemData.Nodes[Logarithm] = Previous->Size.Next;
    }

    if (Previous->Size.Next != NULL) {
      allocationNode* Temp = Previous->Size.Next;
      Temp->Size.Previous = Previous->Size.Previous;
    }

    // (Deal with size-related links, for `Node`)

    if (Node->Size.Previous != NULL) {
      allocationNode* Temp = Node->Size.Previous;
      Temp->Size.Next = Node->Size.Next;
    } else {
      MmSubsystemData.Nodes[Logarithm] = Node->Size.Next;
    }

    if (Node->Size.Next != NULL) {
      allocationNode* Temp = Node->Size.Next;
      Temp->Size.Previous = Node->Size.Previous;
    }

    // (Clear `Previous` and `Node`, so they don't pop up in any
    // future searches)

    Memset((void*)Previous, 0, sizeof(allocationNode));
    Memset((void*)Node, 0, sizeof(allocationNode));

    // -> (2) 'Push' a new node to the level above, although this
    // time using PushBlock().

    Logarithm++;
    PushBlock(Previous, Pointer, Before, After, Logarithm);

    // Finally, let's redefine `Node` to point to our previous
    // block, and repeat the loop.

    Node = Previous;
    continue;

  }

  // (Now that we're done, we can safely return `true`.)

  return true;

}



// (TODO - Kernel memory allocator, malloc(); returns `NULL` if there's
// an issue (size == 0, lack of memory, etc.), and must be contiguous)

[[nodiscard]] void* Allocate(const uintptr* Length) {

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

  auto BlockLevel = MmSubsystemData.Limits[0];

  while (Size > (1ULL << BlockLevel)) {
    BlockLevel++;
  }

  // (If the block level exceeds the maximum limit, just return NULL)

  if (BlockLevel > MmSubsystemData.Limits[1]) {
    return NULL;
  }

  // The allocation system we're using uses doubly-linked lists of nodes,
  // with one list for each 'level' (which corresponds to the amount
  // of memory that node corresponds to).

  // (Essentially, for any N, MmSubsystemData.Nodes[N] is a pointer to the last entry in
  // a list of nodes that correspond to memory areas that are (1 << N)
  // bytes long.)

  // That means there are two steps we need to take:

  // -> (1) If there *aren't* any memory areas at our level, try to
  // find a larger area we can divide)

  if (MmSubsystemData.Nodes[BlockLevel] == NULL) {

    for (auto Level = (BlockLevel + 1); Level <= MmSubsystemData.Limits[1]; Level++) {

      if (MmSubsystemData.Nodes[Level] != NULL) {

        bool Result = DivideBlock(MmSubsystemData.Nodes[Level], Level, BlockLevel);

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

  // -> (2) If possible, merge MmSubsystemData.Nodes[N] with any other nearby
  // blocks (for example, two 4 KiB blocks -> one 8 KiB block).

  return MergeBlock(Node, Size);

}
