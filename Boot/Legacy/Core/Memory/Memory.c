// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "Memory.h"

/* void Memcpy()

   Inputs:  void* Destination - The memory area you want to copy data to.
            void* Source - The memory area you want to copy data from.
            uint32 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function copies a certain amount of data (Size) from one memory area (Source) to another
   (Destination).

   Essentially, it's for when you need to copy/duplicate data - if you want to copy 1000 bytes
   from 20000h to 7C00h for example, you could do Memcpy(0x7C00, 0x20000, 1000).

   Keep in mind that this function doesn't take into account overflows / overlapping memory
   areas; if you want to copy memory between overlapping areas, use Memmove() instead.

*/

void Memcpy(void* Destination, void* Source, uint32 Size) {

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


/* void Memswap()

   Inputs: void* BufferA - The first memory area/buffer that you want to swap.
           void* BufferB - The second memory area/buffer that you want to swap.
           uint32 Size - The size of the memory area/buffer that you want to swap.

   Outputs: (None)

   This function swaps two memory areas - specifically, it swaps the data in BufferA with the
   data in BufferB. It's assumed that both buffers are the same length (Size).

   For example, if you want to swap two elements in an array, you could do:
   - Memswap((void*)&Array[A], (void*)&Array[B], sizeof(Array[0]));

*/

void Memswap(void* BufferA, void* BufferB, uint32 Size) {

  // In order to properly swap these two memory areas, we need to create an auxiliary buffer.

  char Aux[Size];
  void* AuxBuffer = (void*)&Aux[0];

  // Now, we can just swap the two memory areas, using our auxiliary buffer from earlier.

  Memmove(AuxBuffer, BufferA, Size);
  Memmove(BufferA, BufferB, Size);
  Memmove(BufferB, AuxBuffer, Size);

}
