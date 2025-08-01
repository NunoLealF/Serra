// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_SHARED_MEMORY_H
#define SERRA_SHARED_MEMORY_H

  // Standard memory functions (Memcpy, Memmove, etc.), from Memory.c.

  int Memcmp(const void* BufferA, const void* BufferB, uint32 Size);
  void Memcpy(void* Destination, const void* Source, uint32 Size);
  void Memmove(void* Destination, const void* Source, uint32 Size);
  void Memset(void* Buffer, uint8 Character, uint32 Size);
  void Memswap(void* BufferA, void* BufferB, uint32 Size);

  // SSE-optimized memory functions (SseMemcpy, etc.), from Memory.c.
  // (These functions only copy 128 bytes at a time, and addresses must
  // be 16-byte aligned; additionally, SSE *must* be enabled).

  void SseMemcpy(void* Destination, const void* Source, uint32 Size);
  void SseMemset(void* Buffer, uint8 Character, uint32 Size);

  // Even in a freestanding environment, compilers will sometimes still
  // emit calls to memcpy, memcmp, memset and memmove, so we need to
  // provide wrappers for those.

  // (It's true they already exist, but they aren't named identically)

  int memcmp(const void*, const void*, uint32);
  void* memcpy(void*, const void*, uint32);
  void* memmove(void*, const void*, uint32);
  void* memset(void*, int, uint32);

#endif
