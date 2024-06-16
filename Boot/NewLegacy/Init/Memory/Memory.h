// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // A20 functions, from A20/A20_Wrapper.c and A20/A20.s.

  #define A20_TestAddress 0xE000

  extern void EnableKbd_A20(void);
  extern void EnableFast_A20(void);

  bool Check_A20(void);
  void Wait_A20(void);

  // Standard memory functions; memset, memcpy, memmove.., from Memory.c

  int Memcmp(const void* PtrA, const void* PtrB, uint32 Size);
  void Memcpy(void* Destination, void* Source, uint32 Size);
  void Memmove(void* Destination, const void* Source, uint32 Size);
  void Memset(void* Buffer, uint8 Character, uint32 Size);
  void Memswap(void* BufferA, void* BufferB, uint32 Size);

  // (Todo: other functions)

#endif
