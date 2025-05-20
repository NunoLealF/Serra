// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_ENTRYPOINT_STATUS_H
#define SERRA_KERNEL_ENTRYPOINT_STATUS_H

  /* (typedef) enum:uint64{} entrypointReturnValue

     (TODO: Explain what this is for, etc.)

     (TODO: This is guaranteed to be confusing to anyone who doesn't know
     about the intricacies of C enums, definitely explain how the status
     codes are formatted / give a basic overview)

  */

  typedef enum : uint64 {

    // (High bit is 0h - application/OS was loaded successfully)

    entrypointSuccess = (0ULL << 32),

    // (High bit is 1h - issue with the information table header)

    entrypointTablePointerIsNull = (1ULL << 32),

    entrypointTableInvalidSignature,
    entrypointTableInvalidVersion,
    entrypointTableInvalidSize,
    entrypointTableInvalidChecksum,

    // (High bit is 2h - issue with disk- or filesystem-related data)

    entrypointDiskUnsupportedMethod = (2ULL << 32),
    entrypointDiskInvalidData,

    // (High bit is 3h - issue with display-related data)

    entrypointDisplayUnsupportedType = (3ULL << 32),
    entrypointDisplayTooSmall,
    entrypointDisplayInvalidGraphicsData,
    entrypointDisplayInvalidTextData,

    // (High bit is 4h - issue with firmware-related data)

    entrypointFirmwareUnsupportedType = (4ULL << 32),
    entrypointFirmwareInvalidMmapData,
    entrypointFirmwareInvalidBiosData,
    entrypointFirmwareInvalidEfiData,

    // (High bit is 5h - issue with image- or stack-related data)

    entrypointImageUnsupportedType = (5ULL << 32),
    entrypointImageMisalignedStack,
    entrypointImageInvalidStack,
    entrypointImageInvalidExecutable,

    // (High bit is 6h - issue with memory map-related data)

    entrypointMemoryMapIsNull = (6ULL << 32),
    entrypointMemoryMapHasNoEntries,
    entrypointMemoryMapHasOverlappingEntries,
    entrypointMemoryInvalidPreserveOffset,
    entrypointMemoryUnderLimit,

    // (High bit is 7h - issue with system-related data)

    entrypointSystemUnknownArchitecture = (7ULL << 32),
    entrypointSystemUnsupportedArchitecture,
    entrypointSystemInvalidCpuData

  } entrypointReturnStatus;



  /* const char** EntrypointStatusMessage[n]

     Calling convention: [a] -> High bits of entrypointReturnValue
                         [b] -> Low bits of entrypointReturnValue

     Format: char* (must be converted to char16* on EFI systems)

     (TODO: This is 'more' self-explanatory, but explain what it is still)
     (TODO: Provide an example)

  */

  #define GetEntrypointStatus(Value) (EntrypointStatusCodes[Value >> 32][Value & 0xFFFFFFFF])

  const char** EntrypointStatusMessage[8] = {

    (const char*[]){

      "Kernel entrypoint returned successfully."

    },

    (const char*[]){

      "The pointer to the bootloader-provided information table is invalid.",
      "The bootloader-provided information table has an invalid signature.",
      "The bootloader-provided information table has the wrong version.",
      "The bootloader-provided information table has an invalid size.",
      "The bootloader-provided information table has an invalid checksum."

    },

    (const char*[]) {

      "The bootloader uses an unknown or unsupported disk access method.",
      "The bootloader-provided disk information appears to be invalid."

    },

    (const char*[]) {

      "The bootloader uses an unknown or unsupported display type.",
      "The bootloader-provided video mode is too small to be used.",
      "The bootloader-provided graphics data appears to be invalid.",
      "The bootloader-provided text mode data appears to be invalid."

    },

    (const char*[]) {

      "The bootloader uses an unknown or unsupported firmware type.",
      "The bootloader-provided memory map appears to be invalid.",
      "The bootloader-provided BIOS information appears to be invalid.",
      "The bootloader-provided EFI information appears to be invalid."

    },

    (const char*[]) {

      "The kernel uses an unknown or unsupported executable type.",
      "The kernel stack is not aligned to page boundaries.",
      "The kernel stack data appears to be invalid.",
      "The bootloader-provided program data appears to be invalid."

    },

    (const char*[]) {

      "The pointer to the bootloader-provided memory map is invalid.",
      "The bootloader-provided memory map appears to be empty.",
      "The bootloader-provided memory map has overlapping entries.",
      "The bootloader-provided PreserveUntilOffset value is invalid.",
      "The kernel doesn't have enough usable memory to work with."

    },

    (const char*[]) {

      "The bootloader-indicated system architecture is unknown.",
      "The bootloader-indicated system architecture is unsupported.",
      "The bootloader-provided CPU information appears to be invalid.",

    }

  };

#endif
