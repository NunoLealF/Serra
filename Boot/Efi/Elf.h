// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_ELF_H
#define SERRA_ELF_H

  // ELF-related structures, from Elf.c

  typedef struct _elfHeader {

    // (This part is called 'e_ident' in the ELF documentation)

    struct {

      uint32 MagicNumber; // (Must be 464C457Fh).

      uint8 Class; // (Must be 02h for 64-bit.)
      uint8 Endian; // (Must be 01h for little-endian.)
      uint8 Version; // (This should *probably* be at least 01h)

      uint8 Unused[9];

    } __attribute__((packed)) Ident;

    // (The rest of the header)

    uint16 FileType; // (This should *probably* be 02h / executable)
    uint16 MachineType; // (Must be 3Eh for x86_64)
    uint32 Version; // (Same as in ident)

    uint64 Entrypoint; // (The program entrypoint, if one is specified)
    uint64 ProgramHeaderOffset; // (Program header offset, in bytes)
    uint64 SectionHeaderOffset; // (Section header offset, in bytes)

    uint32 Flags; // (Useless on x86)

    uint16 Size; // (ELF header size, in bytes)

    uint16 ProgramHeaderSize; // (Size of a single program header, in bytes)
    uint16 NumProgramHeaders; // (Amount of program headers)

    uint16 SectionHeaderSize; // (Size of a single section header, in bytes)
    uint16 NumSectionHeaders; // (Amount of section headers)

    uint16 StringSectionIndex; // (The index of the string section header)

  } __attribute__((packed)) elfHeader;

  typedef struct _elfProgramHeader {

    uint32 Type; // (0 = ignore, 1 = load, any other = ignore..?)
    uint32 Flags; // (Can be ignored if we're just loading the kernel)
    uint64 Offset; // (The offset where this segment begins, in bytes)

    uint64 VirtAddress; // (The intended virtual address)
    uint64 PhysAddress; // (The intended physical address; can be ignored)

    uint64 Size; // (Copy this)
    uint64 PaddedSize; // (But allocate this)
    uint64 Alignment; // (Unless <= 2, necessary alignment as a power of 2)

  } __attribute__((packed)) elfProgramHeader;

  typedef struct _elfSectionHeader {

    uint32 NameOffset; // (The offset for the name string, in .shstrtab (StringSectionIndex))
    uint32 Type; // (1 = load, 3 = string, otherwise ignore)
    uint64 Flags; // (Can be ignored)

    uint64 Address; // (Same as elfProgramHeader->VirtAddress)
    uint64 Offset; // (The offset from which to start reading from, in bytes)
    uint64 Size; // (The size of the section, also in bytes)

    uint32 Link; // (Can be ignored)
    uint32 Info; // (Can be ignored)

    uint64 Alignment; // (Same definition as in elfProgramHeader)
    uint64 EntrySize; // (If this section is for a table, the size of each entry)

  } __attribute__((packed)) elfSectionHeader;

#endif
