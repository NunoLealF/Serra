// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_FIRMWARE_H
#define SERRA_KERNEL_FIRMWARE_H

  // Import standard and/or necessary headers.

  #include "../Libraries/Stdint.h"
  #include "../Constructors/Firmware/Firmware.h"

  // Include BIOS (E820) memory-map-related definitions.

  typedef enum _biosMmapEntryType : uint32 {

    biosMmapEntryUnknown = 0,
    biosMmapEntryFree = 1,
    biosMmapEntryReserved = 2,
    biosMmapEntryAcpiReclaimable = 3,
    biosMmapEntryAcpiNvs = 4,
    biosMmapEntryUnusable = 5,
    biosMmapEntryDisabled = 6,
    biosMmapEntryPersistent = 7,
    biosMmapEntryUnaccepted = 8,
    biosMmapEntryMax

  } biosMmapEntryType;

  typedef struct _biosMmapEntry {

    uint64 Base;
    uint64 Limit;
    biosMmapEntryType Type;

    uint32 Acpi;

  } __attribute__((packed)) biosMmapEntry;

  #define biosMmapEntrySize (sizeof(biosMmapEntry))
  #define biosMmapEntrySizeWithoutAcpi (biosMmapEntrySize - 4)

  // Include PCI-BIOS-related definitions.
  // (For reference, this is the table returned by (int 1Ah, ax = B101h))

  #define pciBiosInfoTableSignature 0x20494350

  typedef struct _pciBiosInfoTable {

    uint32 Signature;

    uint8 Characteristics;
    uint8 InterfaceLevel[2];

    uint8 LastPciBus;

  } __attribute__((packed)) pciBiosInfoTable;

  // Include EFI-related definitions.

  typedef efiMemoryDescriptor efiMmapEntry;
  #define efiMmapEntrySize (sizeof(efiMmapEntry))

  // Include kernel-specific definitions, and functions from Efi.c (TODO)

#endif
