// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Memory.h"

/* bool Memcmp(), memcmp()

   Inputs: const void* BufferA - The first buffer you want to compare.
           const void* BufferB - The second buffer you want to compare.
           uint32 Size - The size of the buffers you want to compare.

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

bool Memcmp(const void* BufferA, const void* BufferB, uint32 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  const uint8* ByteA = (const uint8*)BufferA;
  const uint8* ByteB = (const uint8*)BufferB;

  // Compare every individual byte in both memory areas, returning false
  // if we find a mismatch.

  for (uint32 i = 0; i < Size; i++) {

    if (ByteA[i] != ByteB[i]) {
      return false;
    }

  }

  // Finally, if we didn't find a single mismatch, return true.

  return true;

}

bool memcmp(const void* BufferA, const void* BufferB, uint32 Size) {
  return Memcmp(BufferA, BufferB, Size);
}



/* void Memcpy()

   Inputs:  void* Destination - The memory area you want to copy data to.
            void* Source - The memory area you want to copy data from.
            uint32 Size - The size of both memory areas, in bytes.

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

void Memcpy(void* Destination, void* Source, uint32 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // Scan through every byte in the source memory area, and copy it to
  // the destination memory area.

  for (uint32 i = 0; i < Size; i++) {
    DestinationByte[i] = SourceByte[i];
  }

}

void* memcpy(void* Destination, const void* Source, uint32 Size) {
  Memcpy(Destination, Source, Size); return Destination;
}



/* void Memmove()

   Inputs:  void* Destination - The memory area you want to move data to.
            const void* Source - The memory area you want to move data from.
            uint32 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function, much like Memcpy(), copies/'moves' a certain amount of data
   (Size) from one memory area (Source) to another (Destination).

   However, unlike Memcpy(), it takes into account overlapping memory areas /
   any possible overflows, making it a safer approach.

   For example, if you want to move 100,000 bytes from 5200h to 5000h (two
   overlapping memory areas), you could do Memmove(0x5000, 0x5200, 100000).

*/

void Memmove(void* Destination, const void* Source, uint32 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // We always assume that the two memory areas overlap, we just don't
  // know where.

  if (Destination < Source) {

    // If Destination > Source, then it's usually safe to just copy
    // normally from the beginning of each memory area, as with Memcpy.

    for (uint32 i = 0; i < Size; i++) {
      DestinationByte[i] = SourceByte[i];
    }

  } else {

    // If Destination <= Source, then we have to copy the data from the end
    // of each memory area, not from the beginning.

    for (uint32 i = Size; i > 0; i--) {
      DestinationByte[(i-1)] = SourceByte[(i-1)];
    }

  }

}

void* memmove(void* Destination, const void* Source, uint32 Size) {
  Memmove(Destination, Source, Size); return Destination;
}



/* void Memset(), memset()

   Inputs:  void* Buffer - The memory area/buffer you want to write to.
            uint8 Character - The character you want to write with.
            uint32 Size - The size of the memory area/buffer.

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

void Memset(void* Buffer, uint8 Character, uint32 Size) {

  // Translate our void* pointer into an uint8* pointer.

  uint8* BufferByte = (uint8*)Buffer;

  // For every byte in the specified memory area/buffer..

  for (uint32 i = 0; i < Size; i++) {

    // Overwrite with the given value (in Character).

    BufferByte[i] = Character;

  }

}

void* memset(void* Buffer, int Character, uint32 Size) {
  Memset(Buffer, (uint8)Character, Size); return Buffer;
}



/* void Memswap()

   Inputs: void* BufferA - The first memory area/buffer that you want to swap.
           void* BufferB - The second memory area/buffer that you want to swap.
           uint32 Size - The size of the memory area/buffer that you want to swap.

   Outputs: (None)

   This function swaps two memory areas - specifically, it swaps the data in
   BufferA with the data in BufferB. It's assumed that both buffers are the
   same length (Size).

   It does this by allocating a temporary auxiliary buffer within the stack,
   so be careful (this stage is limited to 32~64 KiB).

   For example, if you want to swap two elements in an array, you could do:
   -> Memswap((void*)&Array[A], (void*)&Array[B], sizeof(Array[0]));

*/

void Memswap(void* BufferA, void* BufferB, uint32 Size) {

  // In order to properly swap these two memory areas, we need to create an
  // auxiliary buffer.

  char Aux[Size];
  void* AuxBuffer = (void*)&Aux[0];

  // Now, we can just swap the two memory areas, using our auxiliary buffer
  // from earlier.

  Memmove(AuxBuffer, BufferA, Size);
  Memmove(BufferA, BufferB, Size);
  Memmove(BufferB, AuxBuffer, Size);

}



// (TODO: Write documentation / SSE memcpy)

// (!) This function only copies 128 bytes at a time, and addresses
// *must* be 16-byte aligned.

void SseMemcpy(void* Destination, const void* Source, uint32 Size) {

  // Translate each (const) void* pointer into a uint32 address.

  uint32 DestinationAddress = (uint32)Destination;
  uint32 SourceAddress = (uint32)Source;

  // Copy 128 bytes at a time, using XMM registers 0 through 7.
  // (This requires SSE to be enabled - *do not use from Stage2*)

  uint32 NumBlocksLeft = (Size / 128);

  while (NumBlocksLeft > 0) {

    // (Use `movdqa` to move 128-bit (16 byte) blocks at a time into an XMM
    // register, and `movntdq` to move it back into memory)

    __asm__ __volatile__ ("movdqa 0(%%esi), %%xmm0;"
                          "movdqa 16(%%esi), %%xmm1;"
                          "movdqa 32(%%esi), %%xmm2;"
                          "movdqa 48(%%esi), %%xmm3;"
                          "movdqa 64(%%esi), %%xmm4;"
                          "movdqa 80(%%esi), %%xmm5;"
                          "movdqa 96(%%esi), %%xmm6;"
                          "movdqa 112(%%esi), %%xmm7;"
                          "movntdq %%xmm0, 0(%%edi);"
                          "movntdq %%xmm1, 16(%%edi);"
                          "movntdq %%xmm2, 32(%%edi);"
                          "movntdq %%xmm3, 48(%%edi);"
                          "movntdq %%xmm4, 64(%%edi);"
                          "movntdq %%xmm5, 80(%%edi);"
                          "movntdq %%xmm6, 96(%%edi);"
                          "movntdq %%xmm7, 112(%%edi);"
                          :: "S"(SourceAddress), "D"(DestinationAddress) :
                          "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                          "xmm4", "xmm5", "xmm6", "xmm7");

    NumBlocksLeft--;

    SourceAddress += 128;
    DestinationAddress += 128;

  }

  // If there's still anything left to copy, then copy it with the regular
  // (non-SSE) Memcpy.

  if ((Size % 128) != 0) {
    Memcpy((void*)DestinationAddress, (const void*)SourceAddress, (Size % 128));
  }

  // Return.

  return;

}



// (TODO: Write documentation / SSE memset)

// (!) This function only sets 128 bytes at a time, and addresses
// *must* be 16-byte aligned.

void SseMemset(void* Buffer, uint8 Character, uint32 Size) {

  // Create a 128-byte buffer, and fill it with our character of
  // choice using the regular Memset function.

  char Temp[128] __attribute__((aligned(16)));
  Memset(Temp, Character, 128);

  // Using SseMemcpy, copy from TempBuffer as many times as necessary.

  uint32 Address = (uint32)Buffer;
  uint32 Offset = 0;

  while (Offset < Size) {
    SseMemcpy((void*)(Address + Offset), (const void*)Temp, 128);
    Offset += 128;
  }

  // If there's still anything left to set, then set it with the regular
  // (non-SSE) Memset.

  if ((Size % 128) != 0) {
    Memset((void*)(Address + Offset), Character, (Size % 128));
  }

  return;

}
