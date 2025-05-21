// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#if !defined(__amd64__) && !defined(__x86_64__)
  #error "This code must be compiled with an x64 cross compiler."
#endif

#include "../../Libraries/Stdint.h"
#include "x64.h"

// TODO: Global variable that has the list of supported CPU features
// (See definition in x64.h)

cpuFeaturesAvailable CpuFeaturesAvailable_x64 = {false};



/* cpuidRegisterTable QueryCpuid_x64()

   Inputs: uint64 Rax - The rax/eax value you want to call CPUID with.
           uint64 Rcx - The rcx/ecx value you want to call CPUID with.

   Outputs: cpuidRegisterTable{} - A table with the value of the rax,
   rbx, rcx and rdx registers after the call.

   This function essentially serves as a wrapper around the `cpuid`
   instruction - it takes in values for the rax and rcx registers (which
   are equivalent to eax and ecx), and returns a cpuidRegisterTable{}.

   CPUID is guaranteed* to be supported on all 64-bit x86 CPUs, and even
   most 32-bit CPUs; depending on the system, it can support a *wide*
   variety of different instructions, but these should be supported:

   -> (rax = 00000000h) Get the CPU vendor string (in rbx ~ rdx ~ rcx)
   -> (rax = 00000001h) Get the CPU model (rax/rbx) and feature list (rcx/rdx)
   -> (rax = 80000001h) Get more CPU features (in rcx/rdx)

   Additionally, most modern CPUs also support:

   -> (rax = 00000007h) Get additional CPU features (in rbx/rcx/rdx)
   -> (rax = 80000008h) Get address size information (in rax), etc.

   *(In theory, CPUID support isn't actually required for x86-64 at all,
   but in reality it's essentially required for any operating system)

*/

cpuidRegisterTable QueryCpuid_x64(uint64 Rax, uint64 Rcx) {

  // Declare a cpuidRegisterTable{}, and execute the `cpuid` instruction,
  // storing the necessary values in Table.

  cpuidRegisterTable Table = {0};

  __asm__ __volatile__ ("cpuid" : "=a"(Table.Rax), "=b"(Table.Rbx),
                        "=c"(Table.Rcx), "=d"(Table.Rdx) : "a"(Rax), "c"(Rcx));

  // Return that register table.

  return Table;

}



/* void WriteToControlRegister_x64()

   Inputs: uint8 Register - The control register you want to write to (for
           example, 0 writes to CR0, 4 to CR4, etc.); 255 represents XCR0.

           uint64 Value - The value you want to write to the register.

   Outputs: (None, except an updated control register, or a warning message)

   This function attempts to write a value to a writeable control register
   (either CR0, CR3, CR4, CR8 or XCR0), serving as a wrapper around
   `mov $Value, %%cr?` (or `xsetbv`).

   For example, to clear everything in the CR8 register, you could do:
   -> WriteToControlRegister_x64(8, false, 0x00000000);

*/

void WriteToControlRegister_x64(uint8 Register, bool IsExtendedRegister, uint64 Value) {

  // Regular and extended control registers have different access
  // mechanisms (either `mov` or `xsetbv`):

  if (IsExtendedRegister == false) {

    // Depending on the register type, either write value (if possible), or
    // just return silently (CR1, CR2, etc. aren't accessible, for example).

    if (Register == 0) {
      __asm__ __volatile__ ("mov %0, %%cr0" :: "r"(Value));
    } else if (Register == 3) {
      __asm__ __volatile__ ("mov %0, %%cr3" :: "r"(Value));
    } else if (Register == 4) {
      __asm__ __volatile__ ("mov %0, %%cr4" :: "r"(Value));
    } else if (Register == 8) {
      __asm__ __volatile__ ("mov %0, %%cr8" :: "r"(Value));
    }

  } else {

    // If extended control registers are not available, then return early.

    if (CpuFeaturesAvailable_x64.Xsave == false) {
      return;
    }

    // Attempt to write a value using `xsetbv`, which reads 32-bits each
    // from edx:eax, and writes it to the control register at XCR(ecx).

    __asm__ __volatile__ ("xsetbv" :: "a"(Value & 0xFFFFFFFF),
                          "c"(Register), "d"(Value >> 32));

  }

  // Now that we're done, we can return.

  return;

}



/* uint64 ReadFromControlRegister_x64()

   Inputs: uint8 Register - The control register you want to read from (for
           example, 0 writes to CR0, 4 to CR4, etc.);

           bool IsExtendedRegister - Whether you want to read from an
           *extended* control register (like XCR0) or not;

   Outputs: uint64 Value - The value of that control register, or uintmax if
            the control register is unusable (or non-readable).

   This function attempts to read the current value of an accessible control
   register (either CR0, CR2, CR3, CR4, CR8 or XCR0) into Value, serving
   as a wrapper around `mov %%cr?, Value` (or `xgetbv`).

   For example, to read the current value of the XCR0 register, you could do:
   -> uint64 Xcr0 = ReadFromControlRegister_x64(0, true);

*/

uint64 ReadFromControlRegister_x64(uint8 Register, bool IsExtendedRegister) {

  // Regular and extended control registers have different access
  // mechanisms (either `mov` or `xgetbv`):

  if (IsExtendedRegister == false) {

    // Depending on the register type, either read value (if possible), or
    // return uintmax (CR1 and CR5-7 aren't accessible, for example).

    uint64 Value = 0;

    if (Register == 0) {
      __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(Value));
    } else if (Register == 2) {
      __asm__ __volatile__ ("mov %%cr2, %0" : "=r"(Value));
    } else if (Register == 3) {
      __asm__ __volatile__ ("mov %%cr3, %0" : "=r"(Value));
    } else if (Register == 4) {
      __asm__ __volatile__ ("mov %%cr4, %0" : "=r"(Value));
    } else if (Register == 8) {
      __asm__ __volatile__ ("mov %%cr8, %0" : "=r"(Value));
    } else {
      return uintmax;
    }

    // Return that value.

    return Value;

  } else {

    // If extended control registers are not available, then return
    // uintmax.

    if (CpuFeaturesAvailable_x64.Xsave == false) {
      return uintmax;
    }

    // Attempt to read a value using `xgetbv`, which reads 32-bits each
    // to edx:eax, from the control register at XCR(ecx).

    uint32 Value[2] = {0};
    __asm__ __volatile__ ("xgetbv" : "=a"(Value[1]), "=d"(Value[0])
                          : "c"(Register));

    // Reconstruct Value[] (which represents edx and eax) into one
    // cohesive 64-bit value, and return it:

    return ((uint64)Value[0] << 32) | Value[1];

  }

}



/* void WriteToMsr_x64()

   Inputs: uint32 Msr - The MSR number you want to write to.
           uint64 Value - The value you want to update the MSR with.

   Outputs: (none, except new value in MSR)

   This function essentially serves as a wrapper around the wrmsr instruction,
   which updates an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and Value into [edx:eax].

*/

void WriteToMsr_x64(uint32 Msr, uint64 Value) {

  // Execute the wrmsr instruction ([ecx] and [edx:eax] input)

  uint32 Edx = (uint32)(Value >> 32);
  uint32 Eax = (uint32)(Value);

  __asm__ __volatile__ ("wrmsr" :: "a"(Eax), "c"(Msr), "d"(Edx));

  // Now that we've done that, return

  return;

}



/* uint64 ReadFromMsr_x64()

   Inputs: uint32 Msr - The MSR number you want to read from.
   Outputs: uint64 - The value of that MSR.

   This function serves as a wrapper around the rdmsr instruction, which
   reads from an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and the return value comes from [edx:eax].

*/

uint64 ReadFromMsr_x64(uint32 Msr) {

  // Execute the rdmsr instruction ([ecx] input, [edx:eax] output)

  uint32 Eax, Edx;
  __asm__ __volatile__ ("rdmsr" : "=a"(Eax), "=d"(Edx) : "c"(Msr));

  // Assemble that into a proper value, and return

  uint64 Value = ((uint64)Edx << 32) + Eax;
  return Value;

}



// TODO - Function that initializes the CpuFeaturesAvailable global
// variable; this is necessary before we call most constructors.

void InitializeCpuFeatures_x64(void) {

  // First, let's call CPUID with (rax = 00000001h); this should be supported
  // on essentially every x64 processor, and it returns a number of useful
  // feature flags that will be useful for us later on.

  cpuidRegisterTable StandardFeatureFlags = QueryCpuid_x64(1, 0);

  // Additionally, let's also obtain the value of the CR0 and CR4 control
  // registers, since these tell us which features are *enabled*.

  uint64 Cr0 = ReadFromControlRegister_x64(0, false);
  uint64 Cr4 = ReadFromControlRegister_x64(4, false);
  [[maybe_unused]] uint64 Xcr0 = 0;

  // Now that we've done that, let's check which features are actually
  // supported by reading through CPUID:

  #define FxsaveBit (1ULL << 24) // (Within rdx)
  #define XsaveBit (1ULL << 26) // (Within rcx)

  CpuFeaturesAvailable_x64.Fxsave = ((StandardFeatureFlags.Rdx & FxsaveBit) != 0);
  CpuFeaturesAvailable_x64.Xsave = ((StandardFeatureFlags.Rcx & XsaveBit) != 0);

  #define SseBit (1ULL << 25) // (Within rdx)
  #define Sse2Bit (1ULL << 26) // (Within rdx)
  #define Sse3Bit (1ULL << 0) // (Within rcx)
  #define Ssse3Bit (1ULL << 9) // (Within rcx)
  #define Sse4Bit (1ULL << 19) // (Within rcx)
  #define AvxBit (1ULL << 28) // (Within rcx)

  if (CpuFeaturesAvailable_x64.Fxsave == true) {

    CpuFeaturesAvailable_x64.Sse = ((StandardFeatureFlags.Rdx & SseBit) != 0);
    CpuFeaturesAvailable_x64.Sse2 = ((StandardFeatureFlags.Rdx & Sse2Bit) != 0);

    CpuFeaturesAvailable_x64.Sse3 = ((StandardFeatureFlags.Rcx & Sse3Bit) != 0);
    CpuFeaturesAvailable_x64.Ssse3 = ((StandardFeatureFlags.Rcx & Ssse3Bit) != 0);
    CpuFeaturesAvailable_x64.Sse4 = ((StandardFeatureFlags.Rcx & Sse4Bit) != 0);

  }

  if (CpuFeaturesAvailable_x64.Xsave == true) {

    CpuFeaturesAvailable_x64.Avx = ((StandardFeatureFlags.Rcx & AvxBit) != 0);

  }

  // AVX2 and AVX512 feature flags reside in another CPUID leaf, (rax =
  // 00000007h, so if AVX appears to be supported, check that as well.

  if (CpuFeaturesAvailable_x64.Avx == true) {

    // Due to a Windows NT bug, Intel explicitly specifies that the CPUID
    // leaf we're trying to access is only accessible when MISC_ENABLE.LCMV
    // is equal to 0, so let's take care of that first.

    // (MISC_ENABLE is a model-specific register with the ID 000001A0h)
    // (.LCMV is a bit that limits the maximum CPUID leaf)

    #define MiscEnableMsr 0x000001A0
    #define LcmvBit (1ULL << 22) // (Within MISC_ENABLE)

    WriteToMsr_x64(MiscEnableMsr, (ReadFromMsr_x64(MiscEnableMsr) & ~LcmvBit));
    cpuidRegisterTable AvxRelevantFeatureFlags = QueryCpuid_x64(7, 0);

    // Now that we're done, we can analyze the information we were given
    // by CPUID for any relevant AVX feature flags.

    #define Avx2Bit (1ULL << 5) // (Within rbx)
    #define Avx512fBit (1ULL << 16) // (Within rbx)

    CpuFeaturesAvailable_x64.Avx2 = ((AvxRelevantFeatureFlags.Rbx & Avx2Bit) != 0);
    CpuFeaturesAvailable_x64.Avx512f = ((AvxRelevantFeatureFlags.Rbx & Avx512fBit) != 0);

  }

  // Okay - now that we've successfully analyzed the CPU's *supported* feature
  // set, we can move onto enabling any supported features.

  // (Enable `fxsave` *and* SSE instructions, if supported)

  if (CpuFeaturesAvailable_x64.Sse == true) {

    // (Set up CR0, by clearing the EM bit and setting the MP bit)

    #define EmBit (1ULL << 2) // (In cr0)
    #define MpBit (1ULL << 1) // (In cr0)

    Cr0 &= ~EmBit;
    Cr0 |= MpBit;

    WriteToControlRegister_x64(0, false, Cr0);

    // (Set up CR4, by setting the OSFXSR and OSXMMEXCPT bits)

    #define OsFsxrBit (1ULL << 9)
    #define OsXmmExcptBit (1ULL << 10)

    Cr4 |= (OsFsxrBit | OsXmmExcptBit);
    WriteToControlRegister_x64(4, false, Cr4);

    // (Refresh both CR0 and CR4, in case we reuse it later on)

    Cr0 = ReadFromControlRegister_x64(0, false);
    Cr4 = ReadFromControlRegister_x64(4, false);

  }

  // (Enable `xsave` *and* extended control registers, if supported)

  if (CpuFeaturesAvailable_x64.Xsave == true) {

    // (Set up CR4, by setting the OSXSAVE bit)

    #define OsXsaveBit (1ULL << 18)

    Cr4 |= OsXsaveBit;
    WriteToControlRegister_x64(4, false, Cr4);

    // (Refresh CR4)

    Cr4 = ReadFromControlRegister_x64(4, false);

    // (Also set up XCR0, by setting the MMX/FPU and SSE/XMM bits)

    #define FpuBit (1ULL << 0) // (In xcr0)
    #define XmmBit (1ULL << 1) // (In xcr0)

    Xcr0 = ReadFromControlRegister_x64(0, true);

    Xcr0 |= (FpuBit | XmmBit);
    WriteToControlRegister_x64(0, true, Xcr0);

  }

  // (Enable AVX instructions, if supported)

  if (CpuFeaturesAvailable_x64.Avx == true) {

    // (Set up XCR0, by setting the AVX/YMM bit)

    #define YmmBit (1ULL << 2) // (In xcr0)

    Xcr0 |= YmmBit;
    WriteToControlRegister_x64(0, true, Xcr0);

    // (Additionally, if AVX-512f is enabled, set both ZMM bits)

    if (CpuFeaturesAvailable_x64.Avx512f == true) {

      #define OpmaskBit (1ULL << 5) // (In xcr0 - idfk)
      #define ZmmSizeBit (1ULL << 6) // (In xcr0 - enables upper zmm bits)
      #define ZmmAccessBit (1ULL << 7) // (In xcr0 - enables zmm16 ~ zmm31)

      Xcr0 |= (OpmaskBit | ZmmSizeBit | ZmmAccessBit);
      WriteToControlRegister_x64(0, true, Xcr0);

    }

  }

  // Now that we've finished everything, return.

  return;

}
