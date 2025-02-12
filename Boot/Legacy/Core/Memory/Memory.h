// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // A20 functions, from A20/A20_Wrapper.c and A20/A20.s.

  #define A20_TestAddress 0xF000

  extern void EnableKbd_A20(void);
  extern void EnableFast_A20(void);

  bool Check_A20(void);
  void Wait_A20(void);


  // Standard memory functions; memset, memcpy, memmove.., from Memory.c

  void Memcpy(void* Destination, void* Source, uint32 Size);
  void Memmove(void* Destination, const void* Source, uint32 Size);
  void Memset(void* Buffer, uint8 Character, uint32 Size);
  void Memswap(void* BufferA, void* BufferB, uint32 Size);


  // Memory-map-related structures, from Mmap/Mmap.c.

  typedef struct {

    uint64 Base;
    uint64 Limit;
    uint32 Type;

    uint32 Acpi;

  } __attribute__((packed)) mmapEntry;

  #define mmapEntryFree 1
  #define mmapEntryReserved 2
  #define mmapEntryAcpiReclaimable 3
  #define mmapEntryAcpiNvs 4
  #define mmapEntryUnusable 5
  #define mmapEntryDisabled 6
  #define mmapEntryPersistent 7
  #define mmapEntryUnaccepted 8

  // Memory-map-related functions, from Mmap/Mmap.c.

  #define GetEndOfMmapEntry(MmapEntry) (MmapEntry.Base + MmapEntry.Limit)
  uint32 GetMmapEntry(void* Buffer, uint32 Size, uint32 Continuation);

  // Paging- and allocation-related functions, from Paging/Paging.c.
  // (TODO: Do that, I guess)

  uint64 AllocateFromMmap(uint64 Start, uint32 Size, mmapEntry* UsableMmap, uint8 NumUsableMmapEntries);

#endif
