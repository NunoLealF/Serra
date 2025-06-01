// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_MEMORY_H
#define SERRA_KERNEL_MEMORY_H

  // Include standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  // Include structures from Allocator.c

  typedef struct _allocationNode {

    struct _allocationNode* Previous;
    struct _allocationNode* Next;

  } allocationNode;

  // Include functions and global variables from Allocator.c

  extern allocationNode* Nodes[64];
  extern uint8 Levels[2];

  bool InitializeAllocationSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries);

#endif
