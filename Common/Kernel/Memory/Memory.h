// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_MEMORY_H
#define SERRA_KERNEL_MEMORY_H

  // Include standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  // Include functions from Memory.c

  void Memcpy(void* Destination, const void* Source, uint64 Size);
  void Memset(void* Buffer, uint8 Character, uint64 Size);
  void MemsetBlock(void* Buffer, const void* Block, uint64 Size, uint64 BlockSize);

  // Include compiler-required wrappers from Memory.c

  void* memcpy(void*, const void*, uint64);
  void* memset(void*, int, uint64);

  // Include structures from Mm.c

  typedef struct _allocationNode {

    // (Miscellaneous information, and padding)

    void* Pointer; // (What memory block does this node represent?)
    uint8 Padding[24]; // (Not used, for now.)

    // (Position-related information - links to the previous and next
    // nodes, in terms of position)

    struct {

      struct _allocationNode* Previous;
      struct _allocationNode* Next;

    } __attribute__((packed)) Position;

    // (Size-related information - links to the previous and next
    // nodes with the same size)

    struct {

      struct _allocationNode* Previous;
      struct _allocationNode* Next;

    } __attribute__((packed)) Size;

  } __attribute__((packed)) allocationNode;

  static_assert((sizeof(allocationNode) == 64), "`allocationNode`'s size must be a power of two.");

  // Include functions and global variables from Mm.c

  extern bool MemoryManagementEnabled;

  extern allocationNode* Nodes[64];
  extern uint8 Levels[2];

  bool InitializeMemoryManagementSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries);
  bool VerifyMemoryManagementSubsystem(const uintptr Try);

  [[nodiscard]] void* Allocate(const uintptr* Length);
  [[nodiscard]] bool Free(void* Pointer, const uintptr* Length);

#endif
