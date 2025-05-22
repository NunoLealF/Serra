// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#if !defined(__amd64__) && !defined(__x86_64__)
  #error "This code must be compiled with an x64 cross compiler."
#endif

#include "../../Libraries/Stdint.h"
#include "System.h"

/* x64_cpuFeaturesAvailable CpuFeaturesAvailable{}

   Members: (See definition in System.h)

   A table that contains the list of CPU features available on the current
   system, on x64 platforms.

   This table can be accessed by any code that includes System.h, but it's
   only meant to be initialized or modified by InitializeCpuFeatures().

*/

x64_cpuFeaturesAvailable CpuFeaturesAvailable = {false};



/* void InitializeCpuFeatures(void)

   Inputs: (none)
   Outputs: (modifies CpuFeaturesAvailable{})

   (TODO: An introduction like "This is a function that initializes the list
   of available CPU features on x64 platforms")

   (TODO: Something that explains *what* it does, why it's important, etc.)

   (IMPORTANT: When to use this (?))

*/

void InitializeCpuFeatures(void) {

  // First, let's call CPUID with (rax = 00000001h); this should be supported
  // on essentially every x64 processor, and it returns a number of useful
  // feature flags that will be useful for us later on.

  x64_cpuidRegisterTable StandardFeatureFlags = x64_QueryCpuid(1, 0);

  // Additionally, let's also obtain the value of the CR0 and CR4 control
  // registers, since these tell us which features are *enabled*.

  uint64 Cr0 = x64_ReadFromControlRegister(0, false);
  uint64 Cr4 = x64_ReadFromControlRegister(4, false);
  [[maybe_unused]] uint64 Xcr0 = 0;

  // Now that we've done that, let's check which features are actually
  // supported by reading through CPUID:

  #define FxsaveBit (1ULL << 24) // (Within rdx)
  #define XsaveBit (1ULL << 26) // (Within rcx)

  CpuFeaturesAvailable.Fxsave = ((StandardFeatureFlags.Rdx & FxsaveBit) != 0);
  CpuFeaturesAvailable.Xsave = ((StandardFeatureFlags.Rcx & XsaveBit) != 0);

  #define SseBit (1ULL << 25) // (Within rdx)
  #define Sse2Bit (1ULL << 26) // (Within rdx)
  #define Sse3Bit (1ULL << 0) // (Within rcx)
  #define Ssse3Bit (1ULL << 9) // (Within rcx)
  #define Sse4Bit (1ULL << 19) // (Within rcx)
  #define AvxBit (1ULL << 28) // (Within rcx)

  if (CpuFeaturesAvailable.Fxsave == true) {

    CpuFeaturesAvailable.Sse = ((StandardFeatureFlags.Rdx & SseBit) != 0);
    CpuFeaturesAvailable.Sse2 = ((StandardFeatureFlags.Rdx & Sse2Bit) != 0);

    CpuFeaturesAvailable.Sse3 = ((StandardFeatureFlags.Rcx & Sse3Bit) != 0);
    CpuFeaturesAvailable.Ssse3 = ((StandardFeatureFlags.Rcx & Ssse3Bit) != 0);
    CpuFeaturesAvailable.Sse4 = ((StandardFeatureFlags.Rcx & Sse4Bit) != 0);

  }

  if (CpuFeaturesAvailable.Xsave == true) {

    CpuFeaturesAvailable.Avx = ((StandardFeatureFlags.Rcx & AvxBit) != 0);

  }

  // AVX2 and AVX512 feature flags reside in another CPUID leaf, (rax =
  // 00000007h, so if AVX appears to be supported, check that as well.

  if (CpuFeaturesAvailable.Avx == true) {

    // Due to a Windows NT bug, Intel explicitly specifies that the CPUID
    // leaf we're trying to access is only accessible when MISC_ENABLE.LCMV
    // is equal to 0, so let's take care of that first.

    // (MISC_ENABLE is a model-specific register with the ID 000001A0h)
    // (.LCMV is a bit that limits the maximum CPUID leaf)

    #define MiscEnableMsr 0x000001A0
    #define LcmvBit (1ULL << 22) // (Within MISC_ENABLE)

    x64_WriteToMsr(MiscEnableMsr, (x64_ReadFromMsr(MiscEnableMsr) & ~LcmvBit));
    x64_cpuidRegisterTable AvxRelevantFeatureFlags = x64_QueryCpuid(7, 0);

    // Now that we're done, we can analyze the information we were given
    // by CPUID for any relevant AVX feature flags.

    #define Avx2Bit (1ULL << 5) // (Within rbx)
    #define Avx512fBit (1ULL << 16) // (Within rbx)

    CpuFeaturesAvailable.Avx2 = ((AvxRelevantFeatureFlags.Rbx & Avx2Bit) != 0);
    CpuFeaturesAvailable.Avx512f = ((AvxRelevantFeatureFlags.Rbx & Avx512fBit) != 0);

  }

  // Okay - now that we've successfully analyzed the CPU's *supported* feature
  // set, we can move onto enabling any supported features.

  // (Enable `fxsave` *and* SSE instructions, if supported)

  if (CpuFeaturesAvailable.Sse == true) {

    // (Set up CR0, by clearing the EM bit and setting the MP bit)

    #define EmBit (1ULL << 2) // (In cr0)
    #define MpBit (1ULL << 1) // (In cr0)

    Cr0 &= ~EmBit;
    Cr0 |= MpBit;

    x64_WriteToControlRegister(0, false, Cr0);

    // (Set up CR4, by setting the OSFXSR and OSXMMEXCPT bits)

    #define OsFsxrBit (1ULL << 9)
    #define OsXmmExcptBit (1ULL << 10)

    Cr4 |= (OsFsxrBit | OsXmmExcptBit);
    x64_WriteToControlRegister(4, false, Cr4);

    // (Refresh both CR0 and CR4, in case we reuse it later on)

    Cr0 = x64_ReadFromControlRegister(0, false);
    Cr4 = x64_ReadFromControlRegister(4, false);

  }

  // (Enable `xsave` *and* extended control registers, if supported)

  if (CpuFeaturesAvailable.Xsave == true) {

    // (Set up CR4, by setting the OSXSAVE bit)

    #define OsXsaveBit (1ULL << 18)

    Cr4 |= OsXsaveBit;
    x64_WriteToControlRegister(4, false, Cr4);

    // (Refresh CR4)

    Cr4 = x64_ReadFromControlRegister(4, false);

    // (Also set up XCR0, by setting the MMX/FPU and SSE/XMM bits)

    #define FpuBit (1ULL << 0) // (In xcr0)
    #define XmmBit (1ULL << 1) // (In xcr0)

    Xcr0 = x64_ReadFromControlRegister(0, true);

    Xcr0 |= (FpuBit | XmmBit);
    x64_WriteToControlRegister(0, true, Xcr0);

  }

  // (Enable AVX instructions, if supported)

  if (CpuFeaturesAvailable.Avx == true) {

    // (Set up XCR0, by setting the AVX/YMM bit)

    #define YmmBit (1ULL << 2) // (In xcr0)

    Xcr0 |= YmmBit;
    x64_WriteToControlRegister(0, true, Xcr0);

    // (Additionally, if AVX-512f is enabled, set both ZMM bits)

    if (CpuFeaturesAvailable.Avx512f == true) {

      #define OpmaskBit (1ULL << 5) // (In xcr0 - idfk)
      #define ZmmSizeBit (1ULL << 6) // (In xcr0 - enables upper zmm bits)
      #define ZmmAccessBit (1ULL << 7) // (In xcr0 - enables zmm16 ~ zmm31)

      Xcr0 |= (OpmaskBit | ZmmSizeBit | ZmmAccessBit);
      x64_WriteToControlRegister(0, true, Xcr0);

    }

  }

  // Now that we've finished everything, return.

  return;

}
