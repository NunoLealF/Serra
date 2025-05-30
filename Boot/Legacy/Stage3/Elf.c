// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Elf.h"

/* elfProgramHeader* GetProgramHeader()

   Inputs: uintptr Start - The start of the file in memory;
           const elfHeader* ElfHeader - A pointer to the file's ELF header;
           uint16 Index - The index of the program header you want to get.

   Outputs: elfProgramHeader* - A pointer to the program header (if it was
   found), or NULL (if it couldn't be found).

   Each ELF executable is divided into two types of segments, each of which
   have their own specific headers, describing information about that
   specific segment.

   The purpose of this function is to find *program* headers (which are
   ELF headers that describe memory segments, usually executable data),
   according to their index.

   It's assumed that ElfHeader redirects to a valid ELF header, and that
   Index is a valid index number (otherwise, this function will return 0).

*/

elfProgramHeader* GetProgramHeader(uintptr Start, const elfHeader* ElfHeader, uint16 Index) {

  // First, does the index even make sense?

  if (Index >= ElfHeader->NumProgramHeaders) {
    return NULL;
  }

  // Find the address of the program header, and return it as a pointer.

  uintptr Address = (uintptr)(Start + ElfHeader->ProgramHeaderOffset + (Index * ElfHeader->ProgramHeaderSize));
  return (elfProgramHeader*)(Address);

}


/* elfSectionHeader* GetSectionHeader()

   Inputs: uintptr Start - The start of the file in memory;
           const elfHeader* ElfHeader - A pointer to the file's ELF header;
           uint16 Index - The index of the section header you want to get.

   Outputs: elfSectionHeader* - A pointer to the section header (if it was
   found), or NULL (if it couldn't be found).

   Each ELF executable is divided into two types of segments, each of which
   have their own specific headers, describing information about that
   specific segment.

   Unlike the previous function, the purpose of this function is to find
   *section* headers (which are ELF headers that describe sections, usually
   non-executable data), according to their index.

   It's assumed that ElfHeader redirects to a valid ELF header, and that
   Index is a valid index number (otherwise, this function will return 0).

*/

elfSectionHeader* GetSectionHeader(uintptr Start, const elfHeader* ElfHeader, uint16 Index) {

  // First, does the index even make sense?

  if (Index >= ElfHeader->NumSectionHeaders) {
    return NULL;
  }

  // Find the address of the section header, and return it as a pointer.

  uintptr Address = (uintptr)(Start + ElfHeader->SectionHeaderOffset + (Index * ElfHeader->SectionHeaderSize));
  return (elfSectionHeader*)(Address);

}


/* const char* GetElfSectionString()

   Inputs: uintptr Start - The start of the file in memory;

           const elfSectionHeader* StringSection - A pointer to the file's ELF
           section header (you can use GetSectionHeader() for this);

           uint32 NameOffset - The offset of the section name within the
           string section (you can use SectionHeader->NameOffset for this).

   Outputs: const char* - A (non-wide) string that corresponds to the name
   of the ELF section with the given NameOffset.

   Each ELF executable is divided into sections, each of which has a name,
   like ".text", ".data" or ".bss".

   The purpose of this function is to find the *name* of a given section,
   based on the NameOffset field in its header; for example, if you want
   to check if a given section is named ".text", then you could do:

   -> if (Strcmp(GetElfSectionString(Start, StringHeader, GivenSection->NameOffset), ".text") == true) {
   ->   // Do something
   -> }

*/

const char* GetElfSectionString(uintptr Start, const elfSectionHeader* StringSection, uint32 NameOffset) {

  // Are we actually looking at a string section?

  if (StringSection->Type != 0x03) {
    return NULL;
  }

  // Okay - now that we know we are, let's find the offset

  uintptr Address = (uintptr)(Start + StringSection->Offset + NameOffset);
  return (const char*)(Address);

}
