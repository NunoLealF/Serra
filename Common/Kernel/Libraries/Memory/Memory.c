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

  extern void _Memcpy_Base(void* Destination, const void* Source, uint64 Size);
  extern void _Memcpy_Sse2(void* Destination, const void* Source, uint64 Size);

  // Optimized Memset() functions for x64 platforms, from x64/Memset.asm
  // (Width *must* be one of 1, 2, 4 or 8.)

  extern void _Memset_Base(void* Buffer, uint64 Value, uint8 Width, uint64 Size);
  extern void _Memset_Sse2(void* Buffer, uint64 Value, uint8 Width, uint64 Size);

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

    if (CpuFeaturesAvailable.Sse2 == true) {
      _Memcpy_Sse2(Destination, Source, Size);
    } else {
      _Memcpy_Base(Destination, Source, Size);
    }

  #endif

  // Now that we're done, we can just return

  return;

}

void* memcpy(void* Destination, const void* Source, uint64 Size) {
  Memcpy(Destination, Source, Size); return Destination;
}



/* static void _Memset8(), _Memset16(), _Memset32(), _Memset64()

   Inputs: void* Buffer - The buffer you want to write to.
           uint? Value - The value you want to fill with.
           uint64 Size - The size of the buffer, *in bytes*.

   Outputs: (None)

   This group of functions fills a memory area / buffer (Buffer) of a
   certain size (Size) with a specific value (Value), which is sized
   accordingly:

   - _Memset8() only accepts 8-bit (uint8) values for `Value`;
   - _Memset16() only accepts 16-bit (uint16) values for `Value`;
   - _Memset32() only accepts 32-bit (uint32) values for `Value`;
   - _Memset64() only accepts 64-bit (uint64) values for `Value`.

   (TODO: Something about platform-specific optimizations)

   (TODO: Something that explains what happens with misaligned Size
   values (like, what happens if you _Memset64())

*/

void _Memset8(void* Buffer, uint8 Value, uint64 Size) {

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {
      _Memset_Sse2(Buffer, Value, sizeof(Value), Size);
    } else {
      _Memset_Base(Buffer, Value, sizeof(Value), Size);
    }

  #endif

  // Now that we're done, we can just return

  return;

}

void _Memset16(void* Buffer, uint16 Value, uint64 Size) {

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {
      _Memset_Sse2(Buffer, Value, sizeof(Value), Size);
    } else {
      _Memset_Base(Buffer, Value, sizeof(Value), Size);
    }

  #endif

  // Now that we're done, we can just return

  return;

}

void _Memset32(void* Buffer, uint32 Value, uint64 Size) {

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {
      _Memset_Sse2(Buffer, Value, sizeof(Value), Size);
    } else {
      _Memset_Base(Buffer, Value, sizeof(Value), Size);
    }

  #endif

  // Now that we're done, we can just return

  return;

}

void _Memset64(void* Buffer, uint64 Value, uint64 Size) {

  // (Routines specific to x64 platforms)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {
      _Memset_Sse2(Buffer, Value, sizeof(Value), Size);
    } else {
      _Memset_Base(Buffer, Value, sizeof(Value), Size);
    }

  #endif

  // Now that we're done, we can just return

  return;

}



/* macro _Generic::Value Memset(), void* memset()

   Inputs: void* Buffer - The buffer you want to write to.
           uint? Value - The value you want to fill with.
           uint64 Size - The size of the buffer, *in bytes*.

   Outputs: (None, or value of `Buffer`)

   (TODO: Something explaining how this works - more specifically,
   something that explains _Generic()'s function overloading)

   (TODO: Something about GCC's insistence on memset() existing)

*/

#define Memset(Buffer, Value, Size) _Generic((Value), \
                                              uint64: _Memset64, \
                                              uint32: _Memset32, \
                                              uint16: _Memset16, \
                                              default: _Memset8 \
                                            )(Buffer, Value, Size)

void* memset(void* Buffer, uint8 Value, uint64 Size) {
  Memset(Buffer, Value, Size); return Buffer;
}
