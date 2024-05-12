// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // Standard memory functions; memset, memcpy, memmove.., from Memory.c

  int Memcmp(const void* PtrA, const void* PtrB, uint32 Size);
  void Memcpy(void* Destination, void* Source, uint32 Size);
  void Memmove(void* Destination, void* Source, uint32 Size);
  void Memset(void* Buffer, uint8 Character, uint32 Size);
  void Memswap(void* BufferA, void* BufferB, uint32 Size);

  // A20 functions, from A20/A20_C.c and A20/A20_Asm.s.

  bool CheckA20(void);
  void WaitA20(void);

  extern void EnableKbdA20(void);
  extern void EnableFastA20(void);

  // Memory map (E820) structures, from Mmap.c.

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
  #define mmapEntryBad 5
  #define mmapEntryDisabled 6
  #define mmapEntryPersistent 7

  // Memory map (int 12h, int 15h / eax E820h) functions, from Mmap.c.

  uint32 GetMmapEntry(void* Buffer, uint32 Size, uint32 Continuation);
  uint16 GetLowMemory(void);

  // (Todo: other functions)

#endif
