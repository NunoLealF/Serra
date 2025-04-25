// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // A20 functions, from A20/A20_Wrapper.c and A20/A20.s.

  #define A20_TestAddress 0xF000

  extern void EnableA20ByKbd(void);
  extern void EnableA20ByFast(void);

  bool CheckA20(void);
  void WaitA20(void);

  // Standard memory functions from the (shared) Memory folder

  #include "../../Shared/Memory/Memory.h"

  // Memory-map-related structures, from Mmap/Mmap.c.

  typedef struct __mmapEntry {

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

  // (Page-sized, and non-page-sized flags)

  #define pagePresent (1ULL << 0)
  #define pageRw (1ULL << 1)
  #define pageUser (1ULL << 2)
  #define pagePwt (1ULL << 3)
  #define pagePcd (1ULL << 4)
  #define pageAccessed (1ULL << 5)
  #define pageAddress(Addr) ((uint64)Addr & (~0ULL << 12)) // Depending on the page type, upper bits should be reserved
  #define pageXd (1ULL << 63)

  // (Page-sized flags)

  #define pageDirty (1ULL << 6)
  #define ptePat (1ULL << 7) // PTE-only.
  #define pageSize (1ULL << 7) // PS bit; makes a huge page (PDE/PDPE-only)
  #define pageGlobal (1ULL << 8)
  #define pdePat (1ULL << 12) // PDE-only.
  #define pagePk(Key) ((uint64)(Key & 0xF) << 59))

  // (Functions from Paging/Paging.c)

  #define makePageEntry(Addr, Flags) ((uint64)pageAddress(Addr) | (uint64)Flags)
  #define ceilingDivide(Num, Divisor) ((Num + Divisor - 1) / Divisor)

  uint64 PageAlign(uint64 Address);
  uint64 AllocateFromMmap(uint64 Start, uint32 Size, bool Clear, mmapEntry* UsableMmap, uint8 NumUsableMmapEntries);
  uint64 InitializePageEntries(uint64 PhysAddress, uint64 VirtAddress, uint64 Size, uint64* Pml4, uint64 Flags, bool UseLargePages, bool UsePat, uint64 MmapOffset, mmapEntry* UsableMmap, uint16 NumUsableMmapEntries);

#endif
