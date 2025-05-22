// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../String.h"

// (TODO - Write documentation)
// [Platform-agnostic Memcpy - this should work everywhere]

static inline void _Memcpy(void* Destination, const void* Source, uint64 Size) {

  uint64 SourceAddress = (uint64)Source;
  uint64 DestinationAddress = (uint64)Destination;

  while (Size > 0) {

    *(uint8*)(DestinationAddress++) = *(uint8*)(SourceAddress++);
    Size--;

  }

  return;

}



// (TODO - Write documentation)
// [Architecture-specific Memcpy - ...]

#if defined(__amd64__) || defined(__x86_64__)

  // Initialize an area for `fxsave`/`fxrstor` and `xsave`/`xrstor`
  // instructions to save data to.

  volatile uint8 _SimdRegisterArea[3072];

  // Optimized Memcpy() functions for x64 platforms, from x64/Memcpy.asm

  extern void _MemcpyBase(void* Destination, const void* Source, uint64 Size);
  extern void _MemcpySse2(void* Destination, const void* Source, uint64 Size);
  // extern void _Memcpy???(void* Destination, const void* Source, uint64 Size);
  // extern void _MemcpyAvx(void* Destination, const void* Source, uint64 Size);
  // extern void _MemcpyAvx512f(void* Destination, const void* Source, uint64 Size);

#endif



/* void Memcpy(), memcpy()

   Inputs:  void* Destination - The memory area you want to copy data to.
            const void* Source - The memory area you want to copy data from.
            uint64 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function copies a certain amount of data (`Size` bytes) from one
   memory area (Source) to another (Destination).

   (TODO: Something about platform-specific optimizations)

   (TODO: Something about GCC's insistence on memcpy())

   Keep in mind that this function doesn't take into account overflows or
   overlapping memory areas; if you want to copy memory between overlapping
   areas, try Memmove() instead.

*/

void Memcpy(void* Destination, const void* Source, uint64 Size) {

  // This code is platform-agnostic, but it also uses platform-specific
  // optimizations, so we need to run a few checks first.

  // Depending on the platform, run
  // (Check for x64-specific optimizations)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {
      _MemcpySse2(Destination, Source, Size);
    } else {
      _MemcpyBase(Destination, Source, Size);
    }

  #endif

  // (Return.)

  return;

}

void* memcpy(void* Destination, const void* Source, uint64 Size) {
  Memcpy(Destination, Source, Size); return Destination;
}
