// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Cpu.h"

/* uint32 ReadEflags()

   Inputs: (none)
   Outputs: uint32 - The current value of the eflags register

   This function reads the current value of eflags, which is a CPU register that keeps track
   of several important flags (for example, the carry flag, the interrupt enable flag, among
   others).

*/

uint32 ReadEflags(void) {

  uint32 Eflags;
  __asm__ __volatile__ ("pushfl; popl %0" : "=r"(Eflags));

  return Eflags;

}


/* void ChangeEflags()

   Inputs: uint8 Bit - The bit number that you want to change (in the eflags register)
           bool Set - Whether you want to set that bit (true), or clear it (false)

   Outputs: (none)

   This function changes one specific bit in the eflags register. This can be useful for a
   few things - for example, enabling/disabling virtual 8086 mode, or modifying the ID bit
   (which is useful for detecting CPUID support).

   As an example, if you wanted to *set* the carry flag (which is bit 0 in eflags), you
   could do the following:
   -> ChangeEflags(0, true);

*/

void ChangeEflags(uint8 Bit, bool Set) {

  // First, get eflags

  uint32 Eflags = ReadEflags();

  // Next, change the given bit

  if (Set == true) {

    Eflags |= (1 << Bit);

  } else {

    Eflags &= ~(1 << Bit);

  }

  // Now, change eflags
  // The "cc" clobber is to tell the compiler that eflags changes

  __asm__ __volatile__ ("pushfl; movl %0, (%%esp); popfl" :: "r"(Eflags) : "cc");

}


/* bool SupportsCpuid()

   Inputs: (none)
   Outputs: bool - Whether this system supports CPUID (true) or not (false)

   This function checks for the presence of CPUID, by seeing if the ID flag (bit 21 in the
   eflags register) can be modified. This method is guaranteed to work on practically any
   modern system, with the exception of some old Cyrix and NexGen CPUs.

*/

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


/* registerTable GetCpuid()

   Inputs: uint32 Eax - The value of the eax register that you want to call CPUID with.
           uint32 Ecx - The value of the ecx register that you want to call CPUID with.

   Outputs: registerTable - A table with the value of eax, ebx, ecx and edx after the call

   This function has one very simple job - it calls CPUID with the given parameters (the value
   of Eax, and sometimes Ecx), and returns the output of that instruction in a registerTable{}.

   Most CPUID instructions only require an input value for the eax register, but some may
   require one for the ecx register as well, which is why it's included in this function; that
   being said, you can usually just leave Ecx blank.

   As an example, if you wanted to call CPUID with the instruction (eax = 0h, ecx = 0h), you
   could do the following:
   -> registerTable Table = GetCpuid(0x00000000, 0x00000000);

*/

registerTable GetCpuid(uint32 Eax, uint32 Ecx) {

  registerTable Table;
  __asm__ __volatile__ ("cpuid" : "=a"(Table.Eax), "=b"(Table.Ebx), "=c"(Table.Ecx), "=d"(Table.Edx) : "a"(Eax), "c"(Ecx));

  return Table;

}


/* void GetVendorString()

   Inputs: char* Buffer - The buffer you want to store the vendor string in (size > 12 bytes!).
           registerTable Table - The table returned by GetCpuid(0, 0), or CPUID with eax=0.

   Outputs: (none, but Buffer is modified)

   This function uses the data returned by CPUID (with eax = 0h) to get the system's vendor
   ID string, which is stored in ebx ~ edx ~ ecx. Most modern CPUs support this query, and
   depending on the brand, they'll return different values:

   -> Intel processors will generally return "GenuineIntel";
   -> AMD processors will generally return "AuthenticAMD".

   Keep in mind that, even though this function only takes in a char*, the buffer you
   use *must* be at least 13 bytes long, in order to store the vendor string (12 characters)
   and the null byte at the end.

*/

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


/* acpiRsdpTable* GetAcpiRsdpTable()

   Inputs: (none)
   Outputs: acpiRsdpTable* - A pointer to the ACPI RSDP table, if it exists (otherwise, NULL)

   This function tries to find the location of the ACPI Root System Description Pointer (also
   known as the RSDP), using the two methods outlined in the ACPI specification:

   -> First, it tries to find the table within the first 1 KiB of the EBDA;
   -> Second, it may also try to find the table between E0000h and FFFFFh in memory.

   If any of these methods are successful, this function returns a pointer to the RSDP table;
   otherwise, it returns a null pointer (!).

   For reference, the definition of the acpiRsdpTable{} table can be found in Cpu.h, and
   also in the ACPI specification.

*/

acpiRsdpTable* GetAcpiRsdpTable(void) {

  // First, see if we can find it within the first 1 KiB of the EBDA.

  // On most (?) systems, the segment of the address of the EBDA can be located at 40Eh in
  // memory (offset 0Eh from the start of the *regular* BDA); in order to turn it into a
  // proper memory address, we just need to multiply it by 16, like this:

  uint16 EbdaSegment = *(uint16*)(0x40E); // If you're getting an array bounds warning here, it's a GCC optimization bug
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


/* void* GetSmbiosEntryPointTable()

   Inputs: (none)
   Outputs: void* - A pointer to the SMBIOS entry point structure, if found; otherwise, NULL.

   This function searches through the memory area (between F0000h and FFFFFh) to find the
   SMBIOS entry point table. On non-UEFI systems, *which this function is targeted towards*,
   this is as simple as just looking for a 16-bit aligned 'anchor string' within the
   aforementioned memory area.

   That being said, SMBIOS is not available on every system, and if this function fails to
   detect it, it will only return NULL, so make sure to check for that first.

*/

void* GetSmbiosEntryPointTable(void) {

  // On non-UEFI systems, the SMBIOS entry point table can be located by searching for a
  // 16-bit aligned 'anchor string' (basically, a signature) between F0000h and FFFFFh in
  // physical memory.

  // Before we can start searching, we need to define the 32-bit and 64-bit anchor strings
  // that we need to search for, like this:

  #define AnchorString_32b 0x000000005F4D535F
  #define AnchorString_64b 0x0000005F334D535F

  // After that, we can just search for the table between F0000h and FFFFFh in memory,
  // like this:

  for (uint32 Position = 0xF0000; Position < 0x100000; Position += 16) {

    uint64 Reference = *(uint64*)(Position);

    if ((Reference & 0xFFFFFFFF) == AnchorString_32b) {
      return (void*)(Position);
    } else if ((Reference & 0xFFFFFFFFFF) == AnchorString_64b) {
      return (void*)(Position);
    }

  }

  // Finally, if that didn't work, then we can return NULL to indicate that SMBIOS
  // isn't supported.

  return NULL;

}


// ...
// Returns eax, status code; should be 00h, but..

// [TODO - I should rewrite the table, and make it into something that isn't a bitfield *and*
// that's easy to find documentation on; set a bool for each bit basically]

uint32 GetPciBiosInfoTable(pciBiosInfoTable* PciBiosTable) {

  // ...

  realModeTable* Table = InitializeRealModeTable();
  Table->Eax = 0xB101;

  Table->Int = 0x1A;

  RealMode();

  // ...

  PciBiosTable->Signature = Table->Edx;
  PciBiosTable->Characteristics = (uint8)(Table->Eax & 0xFF); // AL; lower 8 bytes

  PciBiosTable->InterfaceLevel[0] = (uint8)((Table->Ebx >> 8) & 0xFF); // BH; higher 8 bytes
  PciBiosTable->InterfaceLevel[1] = (uint8)(Table->Ebx & 0xFF); // BL; lower 8 bytes

  PciBiosTable->LastPciBus = (uint8)(Table->Ecx & 0xFF); // CL; lower 8 bytes

  // ...

  return Table->Eax;

}
