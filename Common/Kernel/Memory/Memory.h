// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_MEMORY_H
#define SERRA_KERNEL_MEMORY_H

  // Include standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  // Include structures from Allocator.c

  typedef struct _allocationNode {

    // (Position-related information: where are we, and where's the
    // previous and next nodes?)

    void* Pointer;

    struct {
      struct _allocationNode* Previous;
      struct _allocationNode* Next;
    } Position;

    // (Size-related position - where's the last block with this size?)

    struct {
      struct _allocationNode* Previous;
    } Size;

  } allocationNode;

  // Include functions and global variables from Allocator.c

  extern allocationNode* Nodes[64];
  extern uint8 Levels[2];

  bool InitializeAllocationSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries);

  [[nodiscard]] void* Malloc(const uintptr* Length);
  [[nodiscard]] bool Free(void* Pointer, const uintptr* Length);

#endif
