// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Memory.h"


/* int Memcmp(), memcmp()

   Inputs: const void* BufferA - The first buffer you want to compare.
           const void* BufferB - The second buffer you want to compare.
           uint64 Size - The size of the buffers you want to compare.

   Outputs: int - Whether the two buffers are equal (0) or not (-1, +1).

   This function compares the contents of two buffers / memory areas of a
   given size, and returns 0 if their contents are equal (and otherwise,
   -1 or +1, depending on the first differing byte).

   The main use of this function within the EFI bootloader is just
   to compare UUIDs; for example:

   -> // (sizeof(efiUuid) == 16)
   -> bool IsSelectedUuid = Memcmp(&TableUuid, &SelectedUuid, 16);

   Please keep in mind that this function is quite slow, since it only
   compares one byte at a time - use SSE instructions for more
   complicated operations.

*/

int Memcmp(const void* BufferA, const void* BufferB, uint64 Size) {

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

int memcmp(const void* BufferA, const void* BufferB, uint64 Size) {
  return Memcmp(BufferA, BufferB, Size);
}



/* void Memcpy(), memcpy()

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

void* memcpy(void* Destination, const void* Source, uint64 Size) {
  Memcpy(Destination, Source, Size); return Destination;
}




/* void Memmove(), memmove()

   Inputs:  void* Destination - The memory area you want to move data to.
            const void* Source - The memory area you want to move data from.
            uint64 Size - The size of both memory areas, in bytes.

   Outputs: (None)

   This function, much like Memcpy(), copies/'moves' a certain amount of data
   (Size) from one memory area (Source) to another (Destination).

   However, unlike Memcpy(), it takes into account overlapping memory areas /
   any possible overflows, making it a safer approach.

   For example, if you want to move 100,000 bytes from 5200h to 5000h (two
   overlapping memory areas), you could do Memmove(0x5000, 0x5200, 100000).

*/

void Memmove(void* Destination, const void* Source, uint64 Size) {

  // Translate each (const) void* pointer into a (const) uint8* pointer.

  uint8* DestinationByte = (uint8*)Destination;
  const uint8* SourceByte = (const uint8*)Source;

  // We always assume that the two memory areas overlap, we just don't
  // know where.

  if (Destination < Source) {

    // If Destination > Source, then it's usually safe to just copy
    // normally from the beginning of each memory area, as with Memcpy.

    for (uint64 i = 0; i < Size; i++) {
      DestinationByte[i] = SourceByte[i];
    }

  } else {

    // If Destination <= Source, then we have to copy the data from the end
    // of each memory area, not from the beginning.

    for (uint64 i = Size; i > 0; i--) {
      DestinationByte[(i-1)] = SourceByte[(i-1)];
    }

  }

}

void* memmove(void* Destination, const void* Source, uint64 Size) {
  Memmove(Destination, Source, Size); return Destination;
}



/* void Memset(), memset()

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

void* memset(void* Buffer, int Character, uint64 Size) {
  Memset(Buffer, (uint8)Character, Size); return Buffer;
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



// (TODO: Write documentation / SSE memcpy)

// (!) This function only copies 256 bytes at a time, and addresses
// *should* be 16-byte aligned.

void SseMemcpy(void* Destination, const void* Source, uint64 Size) {

  // Translate each (const) void* pointer into a uint64 address.

  uint64 DestinationAddress = (uint64)Destination;
  uint64 SourceAddress = (uint64)Source;

  // If our addresses aren't 16-byte aligned, then manually copy the
  // 'remainder' using Memcpy.

  if ((SourceAddress % 16) != 0) {

    uint8 Remainder = (16 - (SourceAddress % 16));
    Memcpy(Destination, Source, Remainder);

    SourceAddress += Remainder;
    DestinationAddress += Remainder;

  }

  // Copy 256 bytes at a time, using XMM registers 0 through 15.
  // (This requires SSE to be enabled.)

  uint64 NumBlocksLeft = (Size / 256);

  while (NumBlocksLeft > 0) {

    // (Use `movdqa` to move 128-bit (16 byte) blocks at a time into an XMM
    // register, and `movntdq` to move it back into memory)

    __asm__ __volatile__ ("movdqa 0(%%rsi), %%xmm0;"
                          "movdqa 16(%%rsi), %%xmm1;"
                          "movdqa 32(%%rsi), %%xmm2;"
                          "movdqa 48(%%rsi), %%xmm3;"
                          "movdqa 64(%%rsi), %%xmm4;"
                          "movdqa 80(%%rsi), %%xmm5;"
                          "movdqa 96(%%rsi), %%xmm6;"
                          "movdqa 112(%%rsi), %%xmm7;"
                          "movdqa 128(%%rsi), %%xmm8;"
                          "movdqa 144(%%rsi), %%xmm9;"
                          "movdqa 160(%%rsi), %%xmm10;"
                          "movdqa 176(%%rsi), %%xmm11;"
                          "movdqa 192(%%rsi), %%xmm12;"
                          "movdqa 208(%%rsi), %%xmm13;"
                          "movdqa 224(%%rsi), %%xmm14;"
                          "movdqa 240(%%rsi), %%xmm15;"
                          "movntdq %%xmm0, 0(%%rdi);"
                          "movntdq %%xmm1, 16(%%rdi);"
                          "movntdq %%xmm2, 32(%%rdi);"
                          "movntdq %%xmm3, 48(%%rdi);"
                          "movntdq %%xmm4, 64(%%rdi);"
                          "movntdq %%xmm5, 80(%%rdi);"
                          "movntdq %%xmm6, 96(%%rdi);"
                          "movntdq %%xmm7, 112(%%rdi);"
                          "movntdq %%xmm8, 128(%%rdi);"
                          "movntdq %%xmm9, 144(%%rdi);"
                          "movntdq %%xmm10, 160(%%rdi);"
                          "movntdq %%xmm11, 176(%%rdi);"
                          "movntdq %%xmm12, 192(%%rdi);"
                          "movntdq %%xmm13, 208(%%rdi);"
                          "movntdq %%xmm14, 224(%%rdi);"
                          "movntdq %%xmm15, 240(%%rdi);"
                          :: "S"(SourceAddress), "D"(DestinationAddress) :
                          "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                          "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
                          "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");

    NumBlocksLeft--;

    SourceAddress += 256;
    DestinationAddress += 256;

  }

  // If there's still anything left to copy, then copy it with the regular
  // (non-SSE) Memcpy.

  if ((Size % 256) != 0) {
    Memcpy((void*)DestinationAddress, (const void*)SourceAddress, (Size % 256));
  }

  // Return.

  return;

}



// (TODO: Write documentation / SSE memset)

// (!) This function only sets 256 bytes at a time, and addresses
// *should* be 16-byte aligned.

void SseMemset(void* Buffer, uint8 Character, uint64 Size) {

  // Create a 256-byte buffer, and fill it with our character of
  // choice using the regular Memset function.

  alignas(16) uint8 Temp[256];
  Memset(Temp, Character, 256);

  // (Declare other variables)

  uint64 Address = (uint64)Buffer;
  uint64 Offset = 0;

  // If our address isn't 16-byte aligned, then Memset the non-
  // -aligned bytes.

  if ((Address % 16) != 0) {

    uint8 Remainder = (16 - (Address % 16));
    Memset(Buffer, Character, Remainder);

    Offset += Remainder;

  }

  // Using SseMemcpy, copy from TempBuffer as many times as necessary.

  while (Offset < Size) {

    SseMemcpy((void*)(Address + Offset), (const void*)Temp, 256);
    Offset += 256;

  }

  // If there's still anything left to set, then set it with the regular
  // (non-SSE) Memset.

  if ((Size % 256) != 0) {
    Memset((void*)(Address + Offset), Character, (Size % 256));
  }

  return;

}
