// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Memory.h"


/* bool Memcmp()

   Inputs: const void* BufferA - The first buffer you want to compare.
           const void* BufferB - The second buffer you want to compare.
           uint64 Size - The size of the buffers you want to compare.

   Outputs: bool - Whether the two buffers are equal or not.

   This function compares the contents of two buffers / memory areas of a
   given size, and returns `true` if their contents are equal (otherwise,
   `false`).

   The main use of this function within the EFI bootloader is just to
   compare UUIDs; for example:

   -> // (sizeof(EfiUuid) == 16)
   -> bool IsGenericTable = Memcmp(&TableUuid, &SelectedUuid, 16);

   Please keep in mind that this function is quite slow, since it only
   compares one byte at a time - use SSE instructions for more complicated
   instructions.

*/

bool Memcmp(const void* BufferA, const void* BufferB, uint64 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  const uint8* ByteA = (const uint8*)BufferA;
  const uint8* ByteB = (const uint8*)BufferB;

  // Compare every individual byte in both memory areas, returning false
  // if we find a mismatch.

  for (uint64 i = 0; i < Size; i++) {

    if (ByteA[i] != ByteB[i]) {
      return false;
    }

  }

  // Finally, if we didn't find a single mismatch, return true.

  return true;

}



/* void Memcpy()

   Inputs:  void* Destination - The memory area you want to copy data to.
            const void* Source - The memory area you want to copy data from.
            uint64 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function copies a certain amount of data (Size) from one memory area
   (Source) to another (Destination).

   Essentially, it's for when you need to copy/duplicate data - if you want
   to copy 1000 bytes from 20000h to 7C00h for example, you could do
   Memcpy(0x7C00, 0x20000, 1000).

   Keep in mind that this function doesn't take into account overflows or
   overlapping memory areas; if you want to copy memory between overlapping
   areas, try Memmove() instead.

*/

void Memcpy(void* Destination, const void* Source, uint64 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // Scan through every byte in the source memory area, and copy it to
  // the destination memory area.

  for (uint64 i = 0; i < Size; i++) {
    DestinationByte[i] = SourceByte[i];
  }

}



/* void Memset()

   Inputs:  void* Buffer - The memory area/buffer you want to write to.
            uint8 Character - The character you want to write with.
            uint64 Size - The size of the memory area/buffer.

   Outputs: (None)

   This function fills a memory area/buffer (Buffer) of a certain size (Size)
   with a specific value (Character), although without any hardware
   acceleration (like MMX, SSE or AVX).

   It's usually used for overwriting/filling buffers - for example, clearing
   video memory, freeing a memory area/block, etc.

   For example, if you want to clear 4000 bytes at B8000h (which should
   clear all characters on screen assuming you're in 80*25 text mode), you
   could do Memset(0xB8000, '\0', 4000).

*/

void Memset(void* Buffer, uint8 Character, uint64 Size) {

  // Translate our void* pointer into an uint8* pointer.

  uint8* BufferByte = (uint8*)Buffer;

  // For every byte in the specified memory area/buffer..

  for (uint64 i = 0; i < Size; i++) {

    // Overwrite with the given value (in Character).

    BufferByte[i] = Character;

  }

}



/* void Memswap()

   Inputs: void* BufferA - The first memory area/buffer that you want to swap.
           void* BufferB - The second memory area/buffer that you want to swap.
           uint64 Size - The size of the memory area/buffer that you want to swap.

   Outputs: (None)

   This function swaps two memory areas - specifically, it swaps the data in
   BufferA with the data in BufferB. It's assumed that both buffers are the
   same length (Size).

   It does this by allocating a temporary auxiliary buffer within the stack,
   so be careful (this stage is usually limited to 128 KiB.)

   For example, if you want to swap two elements in an array, you could do:
   -> Memswap((void*)&Array[A], (void*)&Array[B], sizeof(Array[0]));

*/

void Memswap(void* BufferA, void* BufferB, uint64 Size) {

  // In order to properly swap these two memory areas, we need to create an
  // auxiliary buffer.

  char Aux[Size];
  void* AuxBuffer = (void*)Aux;

  // Now, we can just swap the two memory areas, using our auxiliary buffer
  // from earlier.

  Memcpy(AuxBuffer, BufferA, Size);
  Memcpy(BufferA, BufferB, Size);
  Memcpy(BufferB, AuxBuffer, Size);

}
