// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Elf.h"

// (TODO - Write documentation)

elfProgramHeader* GetProgramHeader(uintptr Start, elfHeader* ElfHeader, uint16 Index) {

  // Does the index make sense?

  if (Index >= ElfHeader->NumProgramHeaders) {
    return NULL;
  }

  // Find the address of the header, and return it as a pointer

  uintptr Address = (uintptr)(Start + ElfHeader->ProgramHeaderOffset + (Index * ElfHeader->ProgramHeaderSize));
  return (elfProgramHeader*)(Address);

}


// (TODO - Write documentation)

elfSectionHeader* GetSectionHeader(uintptr Start, elfHeader* ElfHeader, uint16 Index) {

  // Does the index make sense?

  if (Index >= ElfHeader->NumSectionHeaders) {
    return NULL;
  }

  // Find the address of the header, and return it as a pointer

  uintptr Address = (uintptr)(Start + ElfHeader->SectionHeaderOffset + (Index * ElfHeader->SectionHeaderSize));
  return (elfSectionHeader*)(Address);

}


// (TODO - write documentation)

const char* GetElfSectionString(uintptr Start, elfSectionHeader* StringSection, uint32 Offset) {

  // Are we actually looking at a string section?

  if (StringSection->Type != 0x03) {
    return NULL;
  }

  // Okay - now that we know we are, let's find the offset

  uintptr Address = (uintptr)(Start + StringSection->Offset + Offset);
  return (const char*)(Address);

}
