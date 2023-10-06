// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

/* int Memcmp()

   Inputs:  const void* PtrA - The first memory area to compare with.
            const void* PtrB - The second memory area to compare with.
            uint32 Size - The size of both memory areas, in bytes.

   Outputs: int - A return value that indicates the relationship between the two memory areas.

   This function compares two memory areas/blocks of a given size with each other, and returns
   one of three values: 1 if PtrA > PtrB, 0 if both memory areas are the same, and -1 if PtrB >
   PtrA.

   It's intended to compare data; for example, if you have two memory blocks that you want to
   compare (to see if they're equal to each other), you could use this function to do so.

   Keep in mind that the return value only takes into account the first different known byte,
   not the overall value of the data.

*/

int Memcmp(const void* PtrA, const void* PtrB, uint32 Size) {

  // Translate each const void* pointer into a const uint8* pointer.

  const uint8* ByteA = (const uint8*)PtrA;
  const uint8* ByteB = (const uint8*)PtrB;

  // Scan through each byte in the given memory areas.

  for (uint32 i = 0; i < Size; i++) {

    // If we find a discrepancy (meaning the two memory areas aren't equal), return -1 or
    // 1 respectively (depending on whether PtrA+i is bigger or smaller than PtrB+i).

    if (ByteA[i] < ByteB[i]) {
      return -1;
    } else if (ByteB[i] < ByteA[i]) {
      return 1;
    }

  }

  // If we don't find any discrepancy between the two memory areas, return 0.

  return 0;

}


/* void Memcpy()

   Inputs:  void* Destination - The memory area you want to copy data to.
            const void* Source - The memory area you want to copy data from.
            uint32 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function copies a certain amount of data (Size) from one memory area (Source) to another
   (Destination).

   Essentially, it's for when you need to copy/duplicate data - if you want to copy 1000 bytes
   from 20000h to 7C00h for example, you could do Memcpy(0x7C00, 0x20000, 1000).

   Keep in mind that this function doesn't take into account overflows / overlapping memory
   areas; if you want to copy memory between overlapping areas, use Memmove() instead.

*/

void Memcpy(void* Destination, const void* Source, uint32 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // Scan through every byte in the source memory area, and copy it to the destination
  // memory area.

  for (uint32 i = 0; i < Size; i++) {
    DestinationByte[i] = SourceByte[i];
  }

}


/* void Memmove()

   Inputs:  void* Destination - The memory area you want to move data to.
            const void* Source - The memory area you want to move data from.
            uint32 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function, much like Memcpy(), copies/'moves' a certain amount of data (Size) from one
   memory area (Source) to another (Destination). However, it takes into account overlapping
   memory areas / any possible overflows, making it a safer approach.

   For example, if you want to move 1000 bytes from 5200h to 5000h (two overlapping memory areas),
   you could do Memmove(0x5000, 0x5200, 1000).

*/

void Memmove(void* Destination, const void* Source, uint32 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // We always assume that the two memory areas overlap, we just don't know from where.

  if (Destination < Source) {

    // If Destination > Source, then it's usually safe to just copy normally from the beginning
    // of each memory area, as in Memcpy.

    for (uint32 i = 0; i < Size; i++) {
      DestinationByte[i] = SourceByte[i];
    }

  } else {

    // If Destination <= Source, then we have to copy the data from the end of each memory area,
    // not from the beginning.

    for (uint32 i = Size; i > 0; i--) {
      DestinationByte[(i-1)] = SourceByte[(i-1)];
    }

  }

}


/* void Memset()

   Inputs:  void* Buffer - The memory area/buffer you want to write to.
            uint8 Character - The character you want to write with.
            uint32 Size - The size of the memory area/buffer.

   Outputs: (None)

   This function fills a memory area/buffer (Buffer) of a certain size (Size) with a specific
   value (Character).

   It's usually used for overwriting/filling buffers - for example, clearing video memory,
   freeing a memory area/block, etc.

   For example, if you want to clear 4000 bytes at B8000h (which should clear all characters on
   screen assuming you're in 80x25 text mode), you could do Memset(0xB8000, '\0', 4000).

*/

void Memset(void* Buffer, uint8 Character, uint32 Size) {

  // Translate our void* pointer into an uint8* pointer.

  uint8* BufferByte = (uint8*)Buffer;

  // For every byte in the specified memory area/buffer..

  for (uint32 i = 0; i < Size; i++) {

    // Overwrite with the given value (in Character).

    BufferByte[i] = Character;

  }

}
