// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Graphics/Graphics.h"
#include "Cpu.h"

/* void WriteToMsr()

   Inputs: uint64 Msr - The MSR number you want to write to.
           uint64 Value - The value you want to update the MSR with.

   Outputs: (none, except new value in MSR)

   This function essentially serves as a wrapper around the wrmsr instruction,
   which updates an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and Value into [edx:eax].

*/

void WriteToMsr(uint64 Msr, uint64 Value) {

  // Execute the wrmsr instruction ([ecx] and [edx:eax] input)

  uint64 Edx = (uint64)(Value >> 32);
  uint64 Eax = (uint64)(Value);

  __asm__ __volatile__ ("wrmsr" :: "a"(Eax), "c"(Msr), "d"(Edx));

  // Now that we've done that, return

  return;

}


/* uint64 ReadFromMsr()

   Inputs: uint64 Msr - The MSR number you want to read from.
   Outputs: uint64 - The value of that MSR.

   This function serves as a wrapper around the rdmsr instruction, which
   reads from an MSR (Model Specific Register); more specifically, Msr is
   loaded into [ecx], and the return value comes from [edx:eax].

*/

uint64 ReadFromMsr(uint64 Msr) {

  // Execute the rdmsr instruction ([ecx] input, [edx:eax] output)

  uint64 Eax, Edx;
  __asm__ __volatile__ ("rdmsr" : "=a"(Eax), "=d"(Edx) : "c"(Msr));

  // Assemble that into a proper value, and return

  uint64 Value = ((uint64)Edx << 32) + Eax;
  return Value;

}


/* void WriteToControlRegister()

   Inputs: uint8 Register - The control register you want to write to (for
           example, 0 writes to CR0, 4 to CR4, etc.);

           uint64 Value - The value you want to write to the register.

   Outputs: (None, except an updated control register, or a warning message)

   This function writes a value to a writeable control register (either
   CR0, CR3, CR4 or CR8), and shows a warning message otherwise.

   For example, to clear everything in the CR8 register, you could do:
   -> WriteToControlRegister(8, 0x00000000);

*/

void WriteToControlRegister(uint8 Register, uint64 Value) {

  // Depending on the register type, either write value (if possible), or
  // show a warning (CR1, CR2, etc. aren't accessible, for example).

  if (Register == 0) {
    __asm__ __volatile__ ("mov %0, %%cr0" :: "r"(Value));
  } else if (Register == 3) {
    __asm__ __volatile__ ("mov %0, %%cr3" :: "r"(Value));
  } else if (Register == 4) {
    __asm__ __volatile__ ("mov %0, %%cr4" :: "r"(Value));
  } else if (Register == 8) {
    __asm__ __volatile__ ("mov %0, %%cr8" :: "r"(Value));
  } else {
    Message(Warning, u"Attempted to write to unusable control register (CR%d).", Register);
  }

  // Return.

  return;

}


/* uint64 ReadFromControlRegister()

   Inputs: uint8 Register - The control register you want to read from (for
           example, 0 writes to CR0, 4 to CR4, etc.);

   Outputs: uint64 Value - The value of that control register, or 0 if the
            control register is unusable (which will display warning).

   This function reads the current value of an accessible control register
   (either CR0, CR2, CR3, CR4 or CR8) - returning it in Value - and shows
   a warning message otherwise.

   For example, to read the current value of the CR2 register, you could do:
   -> uint64 Cr2 = ReadFromControlRegister(2);

*/

uint64 ReadFromControlRegister(uint8 Register) {

  // Depending on the register type, either read value (if possible), or
  // show a warning (CR1 and CR5-7 aren't accessible, for example).

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
    Message(Warning, u"Attempted to read from an unusable control register (CR%d)", Register);
  }

  return (uint64)Value;

}


/* uint8 GetCpuProtectionLevel()

   Inputs: (none)

   Outputs: uint8 - The current CPL (CPU protection level, or 'ring') as
   stored in the CS register.

   (TODO: I still need to write documentation, but this should be pretty
   self-explanatory)

*/

uint8 GetCpuProtectionLevel(void) {

  // Read the current value of the CS register.

  uint16 Cs;
  __asm__ __volatile__ ("mov %%cs, %0" : "=r"(Cs));

  // Return the lower two bits (bit 0 and 1, respectively).

  return ((uint8)Cs & 0b11);

}
