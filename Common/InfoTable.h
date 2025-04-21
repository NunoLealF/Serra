// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_INFOTABLE_H
#define SERRA_KERNEL_INFOTABLE_H

  // ...

  #define getInfoTableVersion(Major, Minor) ((Major * 0x100) + Minor)
  #define KernelInfoTableVersion getInfoTableVersion(1, 0)


  // ...

  typedef enum : uint8 {

    Unknown = 0,
    EnabledByDefault = 1,
    EnabledByKbd = 2,
    EnabledByFast = 3

  } A20_MethodType;

  typedef struct {

    A20_MethodType A20_Method;

    // Highest standard CPUID level
    // Highest extended CPUID level

    // PAT supported
    // If so, current MSR value

    // etc.

  } __attribute__((packed)) SystemTable_x86;


  // ...

  typedef enum : uint16 {

    x86 = 0x03,
    Amd64 = 0x3E

  } ArchType;

  typedef struct {

    // (Table section)

    uint64 Signature;
    uint32 Version;
    uint32 Size;

    // (System section)

    struct __System {

      ArchType Architecture;
      uint64 SystemTable; // Pointer to system table; refer to SystemTable_x86

    } __attribute__((packed)) System;

    // (System-independent sections | Memory)
    // (System-independent sections | Filesystem and disk)
    // (System-independent sections | Kernel environment)
    // (System-independent sections | Graphics)

    // (Firmware and external sections)

    // (Other)

    uint16 Checksum;

  } __attribute__((packed)) KernelInfoTable;

  /*
  # Infotable structure

  ## Table sections
  - Signature (`7577757Eh`)
  - Internal table version (can vary, return if not supported)
  - Table size (in bytes)

  ## System-independent sections
  ### System environment
  - System flags (what architecture are we in?)
  - Pointer to **x86-64 info table**
  - - A20 enable method
  - - CPUID data, and highest level
  - - PAT supported, and if so, current MSR value
  - - `...`
  ### Memory
  - Pointer to the (current) top-level page table
  - System-provided memory map (pointer, number of entries)
  - **Filtered/"usable" memory map** (pointer, number of entries)
  - Offset; preserve everything before, use anything after.
  ### Filesystem and disk
  - Method used to access disk
  - - Drive number, if available?
  - Physical and logical bytes per sector
  - Partition type (FAT16, FAT32)
  - Pointer (copy of?) to **BPB table**
  - Initial partition offset, as LBA
  ### Kernel environment
  - Debug flag
  - Pointer to the **kernel ELF header**
  - Pointer to the **start of usable space (80h.low)**
  - Pointer to the **start of driver/module space (E0h.low)**
  - Pointer to the **start of kernel space (F0h.low)**
  ### Graphics
  - What type of graphics (none, text, graphics)
  - Pointer to **text info table (like Terminal_Info)**
  - - Current position, limit, location/buffer, *model*
  - Pointer to **graphics info table**
  - - `...`

  ## Firmware/external sections
  - Firmware flags? (What type was used, etc.)
  - Pointer to **(x86) BIOS info table**
  - - ACPI supported, and if so, pointer to **RSDP and *valid* *SDT**
  - - PCI-BIOS supported, and if so, pointer to table
  - - SMBIOS supported, and if so, pointer to table
  - - Pointer to **real mode wrapper**
  - - Pointer to the **EBDA table**
  - - `...`
  - Pointer to **(x86) UEFI info table**
  - - Something that lets you access the interfaces
  - - - Pointer to the EFI system table, maybe?
  - - `...`

  ## Other
  - A checksum of some sort?
  - `...`
  */

#endif
