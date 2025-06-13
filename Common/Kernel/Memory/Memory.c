// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../System/System.h"
#include "Memory.h"

/* int Memcmp(), memcmp()

   Inputs: const void* BufferA - The first buffer you want to compare.
           const void* BufferB - The second buffer you want to compare.
           uintptr Size - The size of the buffers you want to compare.

   Outputs: int - Whether the two buffers are equal (0) or not (-1, +1).

   This function compares the contents of two buffers / memory areas of a
   given size, and returns 0 if their contents are equal (and otherwise,
   -1 or +1, depending on the first differing byte).

   The main use of this function is to compare variables and memory areas
   that can't be directly compared with the == operator, like UUIDs;
   for example:

   -> bool UuidIsEqual = Memcmp(&Uuid1, &Uuid2, sizeof(genericUuid));

   Please keep in mind that this function is quite slow, since it only
   compares one byte at a time - unlike Memcpy() and Memset(), it
   doesn't use hardware-optimized implementations.

*/

int Memcmp(const void* BufferA, const void* BufferB, uintptr Size) {

  // Translate each const void* pointer into a const uint8* pointer.

  const uint8* ArrayA = (const uint8*)BufferA;
  const uint8* ArrayB = (const uint8*)BufferB;

  // Compare each individual byte across both memory areas, returning
  // -1 or +1 if we find a mismatch.

  for (uintptr Index = 0; Index < Size; Index++) {

    // (-1 means (A[i] < B[i]), whereas 1 means (A[i] > B[i]))

    if (ArrayA[Index] < ArrayB[Index]) {
      return -1;
    } else if (ArrayA[Index] > ArrayB[Index]) {
      return 1;
    }

  }

  // Finally, if we didn't find any mismatch, we can return 0.

  return 0;

}

int memcmp(const void* BufferA, const void* BufferB, uintptr Size) {
  return Memcmp(BufferA, BufferB, Size);
}



// (TODO - Write documentation)
// [Platform-specific Memcpy - for x64 (AMD64 / Intel 64)]

#if defined(__amd64__) || defined(__x86_64__)

  // Optimized Memcpy() functions for x64 platforms, from x64/Memcpy.asm

  extern void Memcpy_RepMovsb(void* Destination, const void* Source, uint64 Size);
  extern void Memcpy_Sse2(void* Destination, const void* Source, uint64 Size);
  extern void Memcpy_Avx(void* Destination, const void* Source, uint64 Size);
  extern void Memcpy_Avx512f(void* Destination, const void* Source, uint64 Size);

  // Optimized Memset() functions for x64 platforms, from x64/Memset.asm

  extern void Memset_RepStosb(void* Buffer, uint8 Value, uint64 Size);
  extern void Memset_Sse2(void* Buffer, uint8 Value, uint64 Size);
  extern void Memset_Avx(void* Buffer, uint8 Value, uint64 Size);
  extern void Memset_Avx512f(void* Buffer, uint8 Value, uint64 Size);

#endif



/* void Memcpy(), void* memcpy()

   Inputs: void* Destination - The memory area you want to copy data to.
           const void* Source - The memory area you want to copy data from.
           uintptr Size - The size of both memory areas, in bytes.

   Outputs: (None, or value of `Destination`)

   This function copies a certain amount of data (`Size` bytes) from one
   memory area (Source) to another (Destination).

   (TODO: Something about platform-specific optimizations)

   (TODO: Something about GCC's insistence on memcpy())

   Keep in mind that this function doesn't take into account overflows or
   overlapping memory areas; if you want to copy memory between overlapping
   areas, try Memmove() instead.

*/

void Memcpy(void* Destination, const void* Source, uintptr Size) {

  // For the most part, our memory copying routines are written in platform-
  // -specific Assembly code, so we need to call different functions
  // depending on the system architecture...

  #if defined(__amd64__) || defined(__x86_64__)

    // (Routines specific to x64 platforms)

    // We take into account feature support, as well as size; in this
    // case, ERMS systems always use `rep movsb`)

    if ((Size < 64) || (CpuFeaturesAvailable.Erms == true)) {

      Memcpy_RepMovsb(Destination, Source, Size);

    } else {

      if (CpuFeaturesAvailable.Avx512f == true) {
        Memcpy_Avx512f(Destination, Source, Size);
      } else if (CpuFeaturesAvailable.Avx == true) {
        Memcpy_Avx(Destination, Source, Size);
      } else {
        Memcpy_Sse2(Destination, Source, Size);
      }

    }

  #else

    // (Routines for other platforms)

    // These are not optimized by default, but should work on any
    // platform this is compiled for.

    uintptr Source = (uintptr)Source;
    uintptr Destination = (uintptr)Destination;
    uintptr Threshold = (Destination + Size);

    while (Destination < Threshold) {

      *(uint8*)Destination = *(uint8*)Source;
      Destination++;

    }

  #endif

  // Now that we're done, we can just return

  return;

}

void* memcpy(void* Destination, const void* Source, uintptr Size) {
  Memcpy(Destination, Source, Size); return Destination;
}



/* void Memmove(), void* memmove()

   Inputs: void* Destination - The memory area you want to move data to.
           const void* Source - The memory area you want to move data from.
           uintptr Size - The size of both memory areas, in bytes.

   (TODO - Something about memmove; this is essentially just a memcpy
   that can deal with overlapping areas.)

   (TODO - Something about the different levels available (for instance,
   if the memory allocation subsystem is available, it can use that)

   (TODO - Something about GCC's insistence on memmove())

*/

void Memmove(void* Destination, const void* Source, uintptr Size) {

  // In order to move overlapping data, we can either manually move
  // each byte (for smaller or non-memory-managed allocations),
  // or allocate a temporary buffer for the move.

  // (If the given size is larger than the system page size, then
  // attempt to allocate a buffer)

  const uintptr MoveSize = Size;
  void* MoveBuffer = NULL;

  if (MmSubsystemData.IsEnabled && (MoveSize >= SystemPageSize)) {
    MoveBuffer = Allocate(&MoveSize);
  }

  // (Depending on whether the buffer was allocated successfully,
  // either move data manually, or into the buffer.)

  if (MoveBuffer != NULL) {

    // (Copy data from `Source` to `MoveBuffer`, and from `MoveBuffer`
    // to `Destination`)

    Memcpy(MoveBuffer, Source, MoveSize);
    Memcpy(Destination, MoveBuffer, MoveSize);

    // (Free `MoveBuffer`, if possible)

    [[maybe_unused]] bool Result = Free(MoveBuffer, &MoveSize);

  } else {

    // (Manually copy each byte from `Source` to `Destination`)

    // If our destination buffer comes *before* our source buffer, then
    // we need to copy each byte back-to-front, not front-to-back.

    uint8* SourceByte = (uint8*)Source;
    uint8* DestinationByte = (uint8*)Destination;

    if (Destination < Source) {

      for (uintptr Index = 0; Index < Size; Index++) {
        DestinationByte[Index] = SourceByte[Index];
      }

    } else {

      for (uintptr Index = Size; Index > 0; Index--) {
        DestinationByte[Index-1] = SourceByte[Index-1];
      }

    }

  }

  // Now that we're done, we can return.

  return;

}

void* memmove(void* Destination, const void* Source, uintptr Size) {
  Memmove(Destination, Source, Size); return Destination;
}



/* void Memset(), void* memset()

   Inputs: void* Buffer - The buffer you want to write to.
           uint8 Character - The character you want to write with.
           uintptr Size - The size of the buffer, *in bytes*.

   Outputs: (None, or value of `Destination`)

   This function fills a memory area/buffer (Buffer) of a certain size (Size)
   with a specific value (Character), although without any hardware
   acceleration (like MMX, SSE or AVX).

   (TODO: Something about platform-specific optimizations)

   (TODO: Something about GCC's insistence on memset() existing)

*/

void Memset(void* Buffer, uint8 Character, uintptr Size) {

  // For the most part, our memory filling routines are written in platform-
  // -specific Assembly code, so we need to call different functions
  // depending on the system architecture...

  #if defined(__amd64__) || defined(__x86_64__)

    // (Routines specific to x64 platforms)

    // We take into account feature support, as well as size; in this
    // case, ERMS systems always use `rep stosb`)

    if ((Size < 64) || (CpuFeaturesAvailable.Erms == true)) {

      Memset_RepStosb(Buffer, Character, Size);

    } else {

      if (CpuFeaturesAvailable.Avx512f == true) {
        Memset_Avx512f(Buffer, Character, Size);
      } else if (CpuFeaturesAvailable.Avx == true) {
        Memset_Avx(Buffer, Character, Size);
      } else {
        Memset_Sse2(Buffer, Character, Size);
      }

    }

  #else

    // (Routines for other platforms)

    // These are not optimized by default, but should work on any
    // platform this is compiled for.

    uintptr Destination = (uintptr)Buffer;
    uintptr Threshold = (Destination + Size);

    while (Destination < Threshold) {

      *(uint8*)Destination = *(uint8*)Block;
      Destination++;

    }

  #endif

  // (Return, now that we're done)

  return;

}

void* memset(void* Buffer, int Value, uintptr Size) {
  Memset(Buffer, (uint8)Value, Size); return Buffer;
}



/* void MemsetBlock()

   Inputs: void* Buffer - The buffer you want to write to.
           void* Block - A pointer to the block you want to write with.
           uintptr Size - The size of the (destination) buffer, *in bytes*.
           uintptr BlockSize - The size of the block you want to write with.

   Outputs: (None)

   (TODO: This is essentially Memset(), but for values (or 'blocks')
   larger than one byte)

   (TODO: Something about block-size-specific optimizations)

*/

void MemsetBlock(void* Buffer, const void* Block, uintptr Size, uintptr BlockSize) {

  // (Routines for block sizes between 1 and 8 bytes)
  // (TODO - This could be optimized further..)

  uintptr Destination = (uintptr)Buffer;
  uintptr Threshold = (Destination + Size);

  if (BlockSize <= 1) {

    // (Use Memset to fill everything in one go.)

    Memset(Buffer, *(uint8*)Block, Size);

  } else if (BlockSize <= 2) {

    // (Manually fill 2 bytes at a time.)

    while (Destination < Threshold) {

      *(uint16*)Destination = *(uint16*)Block;
      Destination += BlockSize;

    }

  } else if (BlockSize <= 4) {

    // (Manually fill 4 bytes at a time.)

    while (Destination < Threshold) {

      *(uint32*)Destination = *(uint32*)Block;
      Destination += BlockSize;

    }

  } else if (BlockSize <= 8) {

    // (Manually fill 8 bytes at a time.)

    while (Destination < Threshold) {

      *(uint64*)Destination = *(uint64*)Block;
      Destination += BlockSize;

    }

  } else {

    // (Manually fill `BlockSize` bytes at a time, with Memcpy)
    // This may be inefficient

    while (Destination < Threshold) {

      Memcpy((void*)Destination, Block, BlockSize);
      Destination += BlockSize;

    }

  }

  // (Return, now that we're done)

  return;

}
