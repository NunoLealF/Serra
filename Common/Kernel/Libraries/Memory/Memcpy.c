// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../String.h"

// (TODO - Write documentation)
// Platform-agnostic Memcpy - this should work everywhere

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
// Optimized Memcpy() functions for use on x64 systems.

#if defined(__amd64__) || defined(__x86_64__)

  /*
  // (...)

  static inline void _MemcpyAvx512f(void* Destination, const void* Source, uint64 Size) {
    return;
  }

  // (...)

  static inline void _MemcpyAvx2(void* Destination, const void* Source, uint64 Size) {
    return;
  }

  // (...)

  static inline void _MemcpyAvx(void* Destination, const void* Source, uint64 Size) {
    return;
  }

  // (...)

  static inline void _MemcpySsse3(void* Destination, const void* Source, uint64 Size) {
    return;
  }
  */

  // (SSE2 - requires 16-byte alignment, uses `prefetchnta` and `movdqa` + `movntdq`)

  static inline void _MemcpySse2(void* Destination, const void* Source, uint64 Size) {

    // (Guarantee that both addresses are 16-byte aligned.)

    uint64 SourceAddress = (uint64)Source;
    uint64 DestinationAddress = (uint64)Destination;

    auto Remainder = (16 - (SourceAddress % 16));

    if (Remainder != 16) {

      _Memcpy(Destination, Source, Remainder);

      SourceAddress += Remainder;
      DestinationAddress += Remainder;
      Size -= Remainder;

    }

    // (Calculate the number of 256-byte blocks, and allocate space for
    // `fxsave` and `fxrstor`)

    uint64 NumBlocks = (Size / 256);
    alignas(64) volatile uint8 FxsaveArea[512];

    // Move each 256-byte block at a time. In this case, we use:

    // -> (1) `fxsave` and `fxrstor` to save and restore the XMM registers;
    // -> (2) `prefetchnta` to prefetch the (temporary) data we're copying;
    // -> (3) `movdqa` and `movntdq` to copy between memory and XMM registers.

    __asm__ __volatile__ ("SaveRegisters%=:"
                            "fxsave (%0);"

                          "CopyEachBlock%=:"

                            "cmp %1, 0;"
                            "je RestoreRegisters%=;"

                            "prefetchnta 256(%%rsi);"
                            "prefetchnta 320(%%rsi);"
                            "prefetchnta 384(%%rsi);"
                            "prefetchnta 448(%%rsi);"

                            "movdqa 0(%%rsi), %%xmm0;"
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

                            "add 256, %%rsi;"
                            "add 256, %%rdi;"
                            "dec %1;"

                            "jmp CopyEachBlock%=;"

                          "RestoreRegisters%=:"
                            "fxrstor (%0);"

                          :: "S"(SourceAddress), "D"(DestinationAddress),
                             "m"(FxsaveArea), "r"(NumBlocks)
                          : "memory", "cc");

    // (Take care of anything that may have been left after the remainder)

    Size -= (NumBlocks * 256);

    if (Size > 0) {

      SourceAddress += (NumBlocks * 256);
      DestinationAddress += (NumBlocks * 256);

      _Memcpy((void*)DestinationAddress, (void*)SourceAddress, Size);

    }

    return;

  }

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

  // (Check for x64-specific optimizations)

  #if defined(__amd64__) || defined(__x86_64__)

    if (CpuFeaturesAvailable.Sse2 == true) {

      _MemcpySse2(Destination, Source, Size);
      return;

    }

  #endif

  // Finally, if none of the code for the above architectures applies to
  // us, then we can try to execute platform-agnostic code.

  // (In theory, this should work on all architectures, and deal with
  // unaligned code relatively well too.)

  _Memcpy(Destination, Source, Size);
  return;

}

void* memcpy(void* Destination, const void* Source, uint64 Size) {

  Memcpy(Destination, Source, Size);
  return Destination;

}
