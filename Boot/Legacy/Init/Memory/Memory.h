// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // Standard memory functions; memset, memcpy, memmove.., from Memory.c

  void Memcpy(void* Destination, void* Source, uint32 Size);
  void Memmove(void* Destination, const void* Source, uint32 Size);
  void Memset(void* Buffer, uint8 Character, uint32 Size);

#endif
