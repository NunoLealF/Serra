// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#if !defined(__amd64__) && !defined(__x86_64__)
  #error "This code must be compiled with an x64 cross compiler."
#endif

#include "../../Libraries/Stdint.h"
#include "x64.h"

/* x64_cpuidRegisterTable x64_QueryCpuid()

   Inputs: uint64 Rax - The rax/eax value you want to call CPUID with.
           uint64 Rcx - The rcx/ecx value you want to call CPUID with.

   Outputs: x64_cpuidRegisterTable{} - A table with the value of the rax,
   rbx, rcx and rdx registers after the call.

   This function essentially serves as a wrapper around the `cpuid`
   instruction - it takes in values for the rax and rcx registers (which
   are equivalent to eax and ecx), and returns a x64_cpuidRegisterTable{}.

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

x64_cpuidRegisterTable x64_QueryCpuid(uint64 Rax, uint64 Rcx) {

  // Declare a x64_cpuidRegisterTable{}, and execute the `cpuid` instruction,
  // storing the necessary values in Table.

  x64_cpuidRegisterTable Table = {0};

  __asm__ __volatile__ ("cpuid" : "=a"(Table.Rax), "=b"(Table.Rbx),
                        "=c"(Table.Rcx), "=d"(Table.Rdx) : "a"(Rax), "c"(Rcx));

  // Return that register table.

  return Table;

}



/* void x64_WriteToControlRegister()

   Inputs: uint8 Register - The control register you want to write to (for
           example, 0 writes to CR0, 4 to CR4, etc.); 255 represents XCR0.

           uint64 Value - The value you want to write to the register.

   Outputs: (None, except an updated control register, or a warning message)

   This function attempts to write a value to a writeable control register
   (either CR0, CR3, CR4, CR8 or XCR0), serving as a wrapper around
   `mov $Value, %%cr?` (or `xsetbv`).

   For example, to clear everything in the CR8 register, you could do:
   -> x64_WriteToControlRegister(8, false, 0x00000000);

*/

void x64_WriteToControlRegister(uint8 Register, bool IsExtendedRegister, uint64 Value) {

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

    if (CpuFeaturesAvailable.Xsave == false) {
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



/* uint64 x64_ReadFromControlRegister()

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
   -> uint64 Xcr0 = x64_ReadFromControlRegister(0, true);

*/

uint64 x64_ReadFromControlRegister(uint8 Register, bool IsExtendedRegister) {

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

    if (CpuFeaturesAvailable.Xsave == false) {
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



/* void x64_WriteToMsr()

   Inputs: uint32 Msr - The MSR number you want to write to.
           uint64 Value - The value you want to update the MSR with.

   Outputs: (none, except new value in MSR)

   This function essentially serves as a wrapper around the wrmsr instruction,
   which updates an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and Value into [edx:eax].

*/

void x64_WriteToMsr(uint32 Msr, uint64 Value) {

  // Execute the wrmsr instruction ([ecx] and [edx:eax] input)

  uint32 Edx = (uint32)(Value >> 32);
  uint32 Eax = (uint32)(Value);

  __asm__ __volatile__ ("wrmsr" :: "a"(Eax), "c"(Msr), "d"(Edx));

  // Now that we've done that, return

  return;

}



/* uint64 x64_ReadFromMsr()

   Inputs: uint32 Msr - The MSR number you want to read from.
   Outputs: uint64 - The value of that MSR.

   This function serves as a wrapper around the rdmsr instruction, which
   reads from an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and the return value comes from [edx:eax].

*/

uint64 x64_ReadFromMsr(uint32 Msr) {

  // Execute the rdmsr instruction ([ecx] input, [edx:eax] output)

  uint32 Eax, Edx;
  __asm__ __volatile__ ("rdmsr" : "=a"(Eax), "=d"(Edx) : "c"(Msr));

  // Assemble that into a proper value, and return

  uint64 Value = ((uint64)Edx << 32) + Eax;
  return Value;

}
