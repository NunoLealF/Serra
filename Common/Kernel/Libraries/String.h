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

    // Include standard library functions

    void Memcpy(void* Destination, const void* Source, uintptr Size);
    void Memmove(void* Destination, const void* Source, uintptr Size);
    void Memset(void* Buffer, uint8 Character, uintptr Size);

    // Include non-standard functions

    void MemsetBlock(void* Buffer, const void* Block, uintptr Size, uintptr BlockSize);

    // Include compiler-required wrappers; keep in mind that function
    // names are case-sensitive, so this is needed.

    void* memcpy(void*, const void*, uintptr);
    void* memmove(void*, const void*, uintptr);
    void* memset(void*, int, uint64);

  #endif

#endif
