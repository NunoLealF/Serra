// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Cpu.h"


// First, we need to check to see if CPUID is even available

// There are two ways of doing this - either we can check for the ID flag in EFLAGS (doesn't
// work with a few CPUs from the 90s, whatever), or, we can just.. run it lol.

// Given that CPUID is basically necessary, and that the only CPUs that don't really support
// it are Cyrixes from a pretty long time ago, let's just.. check for it the normal way, lol.


// [TODO - I'm *really* not sure whether these functions are safe, honestly. They work just
// fine on my machine, but they could be a huge issue on another one]

// ...

uint32 ReadEflags(void) {

  uint32 Eflags;
  __asm__ __volatile__ ("pushfl; popl %0" : "=r"(Eflags));

  return Eflags;

}

// ...

void ChangeEflags(uint8 Bit, bool Set) {

  // First, get eflags

  uint32 Eflags = ReadEflags();

  // Next, change the bit

  if (Set == true) {

    Eflags |= (1 << Bit);

  } else {

    Eflags &= ~(1 << Bit);

  }

  // Now, change eflags
  // The "cc" clobber is to tell the compiler that eflags changes

  __asm__ __volatile__ ("pushfl; movl %0, (%%esp); popfl" :: "r"(Eflags) : "cc");

}



// ...

bool SupportsCpuid(void) {

  // First, read the 'original'/'old' eflags, and isolate the CPUID bit, before
  // inverting it with ChangeEflags()

  uint32 Eflags = ReadEflags();

  if ((Eflags & CpuidFlag) == 0) {

    ChangeEflags(CpuidBit, true);

  } else {

    ChangeEflags(CpuidBit, false);

  }

  // Now, we want to compare the value of our previous eflags register with the current
  // one (after we already inverted the CPUID bit).

  if ((Eflags & CpuidFlag) != (ReadEflags() & CpuidFlag)) {
    return true;
  } else {
    return false;
  }

}

// ...

// (Eax and ecx is only supported because there's a few instructions that use ecx, but
// for the most part it's usually useless)

registerTable GetCpuid(uint32 Eax, uint32 Ecx) {

  registerTable Table;
  __asm__ __volatile__ ("cpuid" : "=a"(Table.Eax), "=b"(Table.Ebx), "=c"(Table.Ecx), "=d"(Table.Edx) : "a"(Eax), "c"(Ecx));

  return Table;

}

// ...

void GetVendorString(char* Buffer, registerTable Table) {

  // Now, just copy the vendor string to the given buffer.

  // Because of how the string is laid out, we'll be using a for loop instead of something
  // like Memcpy():

  for (uint8 Offset = 0; Offset < 4; Offset++) {

    Buffer[Offset + 0] = ((Table.Ebx >> (Offset * 8)) & 0xFF);
    Buffer[Offset + 4] = ((Table.Edx >> (Offset * 8)) & 0xFF);
    Buffer[Offset + 8] = ((Table.Ecx >> (Offset * 8)) & 0xFF);

  }

  // Finally, we can append a null byte to the end of the string to terminate it there

  Buffer[12] = '\0';

}




// ...
// (get the ACPI table (RSDP))

acpiRsdpTable* GetAcpiRsdpTable(void) {

  // First, see if we can find it within the first 1 KiB of the EBDA.

  // On most (?) systems, the segment of the address of the EBDA can be located at 40Eh in
  // memory (offset 0Eh from the start of the *regular* BDA); in order to turn it into a
  // proper memory address, we just need to multiply it by 16, like this:

  uint16 EbdaSegment = *(uint16*)(0x40E);
  uint32 EbdaAddress = (uint32)(EbdaSegment << 4);

  // Assuming that address isn't zero, we can then start looking for the EBDA. Since it's
  // only found on 16-byte boundaries, and always starts with the signature 'RSD PTR ', we
  // can just look for it like this:

  uint64 Signature = 0x2052545020445352;

  if (EbdaAddress != 0) {

    for (unsigned int Offset = 0; Offset <= 1024; Offset += 16) {

      if (Signature == *(uint64*)(EbdaAddress + Offset)) {
        return (acpiRsdpTable*)(EbdaAddress + Offset);
      }

    }

  }

  // If we didn't find it using hte last method, then we need to look through the (read-only) area
  // between E0000h and FFFFFh in memory, using the same logic as before:

  for (unsigned int Position = 0xE0000; Position < 0x100000; Position += 16) {

    if (Signature == *(uint64*)(Position)) {
      return (acpiRsdpTable*)(Position);
    }

  }

  // Otherwise, we just need to return NULL.

  return NULL;

}
