// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EFI_MEMORY_H
#define SERRA_EFI_MEMORY_H

  // Standard memory functions (Memcmp, Memset, etc.), from Memory.c

  bool Memcmp(const void* BufferA, const void* BufferB, uint64 Size);
  void Memcpy(void* Destination, const void* Source, uint64 Size);
  void Memset(void* Buffer, uint8 Character, uint64 Size);

#endif
