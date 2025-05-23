// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [64-bit String.h - do not use with 16- or 32-bit code]
// [Relies on kernel- and platform-specific headers]

#ifndef SERRA_KERNEL_STRING_H
#define SERRA_KERNEL_STRING_H

  // Include standard and/or necessary headers.

  #include "Stdint.h"
  #include "../Constructors/System/System.h"

  // Include functions provided by Memory/Memcpy.c (TODO)

  void Memcpy(void* Destination, const void* Source, uint64 Size);
  void* memcpy(void* Destination, const void* Source, uint64 Size);

  // Include functions provided by Memory/Memset.c (TODO)

  void _Memset8(void* Buffer, uint8 Value, uint64 Size);
  void _Memset16(void* Buffer, uint16 Value, uint64 Size);
  void _Memset32(void* Buffer, uint32 Value, uint64 Size);
  void _Memset64(void* Buffer, uint64 Value, uint64 Size);

  #define Memset(Buffer, Value, Size) _Generic((Value), \
                                                uint64: _Memset64, \
                                                uint32: _Memset32, \
                                                uint16: _Memset16, \
                                                default: _Memset8 \
                                              )(Buffer, Value, Size)

  void* memset(void* Buffer, uint8 Character, uint64 Size);

#endif
