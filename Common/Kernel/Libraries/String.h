// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_STRING_H
#define SERRA_KERNEL_STRING_H

  // Include standard and/or necessary headers.

  #include "Stdint.h"

  // Include formatting and string-related functions from String.c

  int Strlen(const char* String);
  int StrlenWide(const char16* String);

  char* Strrev(char* String);
  char* Itoa(uint64 Number, char* Buffer, uint8 Base);

  // Include memory-related functions from Memory/Memory.c

  #ifndef SERRA_KERNEL_MEMORY_H

    void Memcpy(void* Destination, const void* Source, uint64 Size);
    void Memset(void* Buffer, uint8 Character, uint64 Size);
    void MemsetBlock(void* Buffer, const void* Block, uint64 Size, uint64 BlockSize);

    // Even in a freestanding environment, compilers will sometimes still
    // emit calls to memcpy, memcmp, memset and memmove, so we need to
    // provide wrappers for those.

    // (It's true they already exist, but they aren't named identically)

    void* memcpy(void*, const void*, uint64);
    void* memset(void*, int, uint64);

  #endif

#endif
