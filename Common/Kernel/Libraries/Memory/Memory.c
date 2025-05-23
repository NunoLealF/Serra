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

  extern void _Memcpy_RepMovsb(void* Destination, const void* Source, uint64 Size);
  extern void _Memcpy_Sse2(void* Destination, const void* Source, uint64 Size);
  extern void _Memcpy_Avx(void* Destination, const void* Source, uint64 Size);
  extern void _Memcpy_Avx512f(void* Destination, const void* Source, uint64 Size);

  // Optimized Memset() functions for x64 platforms, from x64/Memset.asm
  // (Width *must* be one of 1, 2, 4 or 8.)

  extern void _Memset_RepStosb(void* Buffer, uint64 Value, uint64 Size);
  extern void _Memset_Sse2(void* Buffer, uint64 Value, uint64 Size);
  extern void _Memset_Avx(void* Buffer, uint64 Value, uint64 Size);
  extern void _Memset_Avx512f(void* Buffer, uint64 Value, uint64 Size);

#endif



/* void Memcpy(), void* memcpy()

   Inputs: void* Destination - The memory area you want to copy data to.
           const void* Source - The memory area you want to copy data from.
           uint64 Size - The size of both memory areas, in bytes.

   Outputs: (None, or value of `Destination`)

   This function copies a certain amount of data (`Size` bytes) from one
   memory area (Source) to another (Destination).

   (TODO: Something about platform-specific optimizations)

   (TODO: Something about GCC's insistence on memcpy())

   Keep in mind that this function doesn't take into account overflows or
   overlapping memory areas; if you want to copy memory between overlapping
   areas, try Memmove() instead.

*/

void Memcpy(void* Destination, const void* Source, uint64 Size) {

  // For the most part, our memory copying routines are written in platform-
  // -specific Assembly code, so we need to call different functions
  // depending on the system architecture...

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if ((Size < 1024) || (CpuFeaturesAvailable.Erms == true)) {

      _Memcpy_RepMovsb(Destination, Source, Size);

    } else {

      if (CpuFeaturesAvailable.Avx512f == true) {
        _Memcpy_Avx512f(Destination, Source, Size);
      } else if (CpuFeaturesAvailable.Avx == true) {
        _Memcpy_Avx(Destination, Source, Size);
      } else {
        _Memcpy_Sse2(Destination, Source, Size);
      }

    }

  #endif

  // Now that we're done, we can just return

  return;

}

void* memcpy(void* Destination, const void* Source, uint64 Size) {
  Memcpy(Destination, Source, Size); return Destination;
}



/* static void Memset(), void* memset()

   Inputs: void* Buffer - The buffer you want to write to.
           uint8 Character - The character you want to write with.
           uint64 Size - The size of the buffer, *in bytes*.

   Outputs: (None, or value of `Destination`)

   This function fills a memory area/buffer (Buffer) of a certain size (Size)
   with a specific value (Character), although without any hardware
   acceleration (like MMX, SSE or AVX).

   (TODO: Something about platform-specific optimizations)

   (TODO: Something about GCC's insistence on memset() existing)

*/

void Memset(void* Buffer, uint8 Character, uint64 Size) {

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if ((Size < 1024) || (CpuFeaturesAvailable.Erms == true)) {

      _Memset_RepStosb(Buffer, Character, Size);

    } else {

      if (CpuFeaturesAvailable.Avx512f == true) {
        _Memset_Avx512f(Buffer, Character, Size);
      } else if (CpuFeaturesAvailable.Avx == true) {
        _Memset_Avx(Buffer, Character, Size);
      } else {
        _Memset_Sse2(Buffer, Character, Size);
      }

    }

  #endif

  // (Return, now that we're done)

  return;

}

void* memset(void* Buffer, uint8 Value, uint64 Size) {
  Memset(Buffer, Value, Size); return Buffer;
}
