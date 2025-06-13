// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_MEMORY_H
#define SERRA_KERNEL_MEMORY_H

  // Include standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  // Include standard library functions from Memory.c

  int Memcmp(const void* BufferA, const void* BufferB, uintptr Size);
  void Memcpy(void* Destination, const void* Source, uintptr Size);
  void Memmove(void* Destination, const void* Source, uintptr Size);
  void Memset(void* Buffer, uint8 Character, uintptr Size);

  // Include non-standard library functions from Memory.c

  void MemsetBlock(void* Buffer, const void* Block, uintptr Size, uintptr BlockSize);

  // Include compiler-required wrappers from Memory.c

  int memcmp(const void*, const void*, uintptr);
  void* memcpy(void*, const void*, uintptr);
  void* memset(void*, int, uintptr);
  void* memmove(void*, const void*, uintptr);

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

  typedef struct _mmSubsystemData {

    bool IsEnabled; // Is the memory management subsystem currently enabled?

    allocationNode* Nodes[64]; // A list of heads to linked lists for different sizes
    uint8 Limits[2]; // The lower and upper bounds/limits for Nodes[]

  } mmSubsystemData;

  // Include functions and global variables from Mm.c

  extern mmSubsystemData MmSubsystemData;

  bool InitializeMemoryManagementSubsystem(void* UsableMmap, uint16 NumUsableMmapEntries);
  bool VerifyMemoryManagementSubsystem(const uintptr Try);

  [[nodiscard]] void* Allocate(const uintptr* Length);
  [[nodiscard]] bool Free(void* Pointer, const uintptr* Length);

#endif
