// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_COMMON_H
#define SERRA_COMMON_H

  /* (typedef) union uptr{}

     Members: uint64 Address - The address being pointed to;
              void* Pointer - Same as Address, but in pointer form.

     One of the main problems with sharing the same data structures between
     32- and 64-bit code is that the width of a pointer - and, consequently,
     the size of void* - varies.

     The purpose of this union type is to let 32-bit code deal with 64-bit
     pointers - for example, if you wanted to pass a pointer to a 64-bit
     address while still in 32-bit mode, you could do:

     -> Example.Address = 0x123456789; // (In 32-bit mode)
     -> void* ExamplePtr = Example.Pointer // (In 64-bit mode)

*/

  typedef union __uptr {

    uint64 Address;
    void* Pointer;

  } uptr;

  static_assert((sizeof(uptr) == 8), "`uptr` is not 8 bytes");



  /* (typedef) struct usableMmapEntry{}

     Members: uint64 Base - The starting address of this memory map entry;
              uint64 Limit - The size (or 'limit') of this entry, in bytes.

     (TODO: Something about how this is for the usable mmap (at least,
     for everything after PreserveUntilOffset))

  */

  typedef struct _usableMmapEntry {

    uint64 Base;
    uint64 Limit;

  } __attribute__((packed)) usableMmapEntry;

  static_assert((sizeof(usableMmapEntry) == 16), "`usableMmapEntry` is not 16 bytes");



  /* struct attribute__((packed)) commonInfoTable{}

     Location: (Depends, but probably somewhere in the bootloader stack)
     Members: (...)

     When our bootloader transfers control to the kernel (or our 'common
     stage'), it also passes some important information along with it,
     by passing *this* table as a parameter to the kernel entrypoint.

     Because of that, it has to contain *everything* the kernel needs to
     know in order to set up the boot manager environment, essentially
     functioning like a boot protocol.

     Not only that, but it also has to be platform-independent; we'll
     still have to rely on firmware-specific protocols for some things,
     but everything else should be the same across all platforms.

     (!) (See documentation at InfoTableFormat.md)

     (TODO: Explain this better, and properly document each member; in
     theory this should be 'self-documenting', but its pretty complicated)

     (TODO: Explain security mechanisms; verifying the checksums, sanity-
     -checking table signatures, checking for null pointers, etc.)

  */

  #define commonInfoTableSignature 0x7577757E7577757E
  #define commonInfoTableVersion 5

  typedef struct _commonInfoTable {

    // [Table header]

    uint64 Signature;
    uint16 Version;
    uint16 Size;

    // [Disk and filesystem-related information]

    struct {

      enum : uint16 {

        UnknownMethod = 0,
        EfiFsMethod = 1,
        Int13Method = 2,
        MaxMethod

      } AccessMethod;

      struct {

        uptr FileInfo;

        uptr HandleList;
        uptr Handle;
        uptr Protocol;

      } __attribute__((packed)) EfiFs;

      struct {

        uint8 DriveNumber;

        struct {

          bool IsEnabled;
          uptr Table;

        } __attribute__((packed)) Edd;

      } __attribute__((packed)) Int13;

    } __attribute__((packed)) Disk;

    // [Display and graphics-related information]

    struct {

      enum : uint16 {

        UnknownDisplay = 0,
        VgaDisplay = 1,
        VbeDisplay = 2,
        EfiTextDisplay = 3,
        GopDisplay = 4,
        MaxDisplay

      } Type;

      struct {

        bool IsSupported;

        uint16 PreferredResolution[2];
        uptr Table;

      } __attribute__((packed)) Edid;

      struct {

        uptr Framebuffer;
        uint32 Pitch;
        uint16 LimitX;
        uint16 LimitY;

        struct {

          uint8 PerPixel;

          uint64 RedMask;
          uint64 GreenMask;
          uint64 BlueMask;

        } __attribute__((packed)) Bits;

      } __attribute__((packed)) Graphics;

      struct {

        enum : uint8 {

          UnknownFormat = 0,
          AsciiFormat = 1,
          Utf16Format = 2,
          MaxFormat

        } Format;

        uint16 XPos; // (optional for EfiTextDisplayType)
        uint16 YPos; // (optional for EfiTextDisplayType)

        uint16 LimitX;
        uint16 LimitY;

      } __attribute__((packed)) Text;

    } __attribute__((packed)) Display;

    // [Firmware-related information]

    struct {

      enum {

        UnknownFirmware = 0,
        BiosFirmware = 1,
        EfiFirmware = 2,
        MaxFirmware

      } Type;

      struct {

        enum : uint8 {

          EnabledByUnknown = 0,
          EnabledByDefault = 1,
          EnabledByKeyboard = 2,
          EnabledByFast = 3,
          EnabledByMax

        } A20;

        struct {

          uint16 NumEntries;
          uint32 EntrySize;
          uptr List; // (pointer to a list of e820MmapEntry{})

        } __attribute__((packed)) Mmap;

        struct {

          bool IsSupported;
          uint64 Value;

        } __attribute__((packed)) Pat;

        struct {

          bool IsSupported;
          uptr Table;

        } __attribute__((packed)) PciBios;

        struct {

          bool IsSupported;
          uptr InfoBlock;
          uptr ModeInfo;
          uint16 ModeNumber;

        } __attribute__((packed)) Vbe;

      } Bios;

      struct {

        uptr ImageHandle;
        uptr SystemTable;

        bool SupportsConIn;
        bool SupportsConOut;

        struct {

          bool IsSupported;
          uptr Protocol;

        } __attribute__((packed)) Gop;

        struct {

          uint16 NumEntries;
          uint64 EntrySize;
          uptr List; // (pointer to a list of efiMmapEntry{})

        } __attribute__((packed)) Mmap;

      } Efi;

    } __attribute__((packed)) Firmware;

    // [Image-related information]

    struct {

      uptr StackTop; // (StackStart + StackSize)
      uint64 StackSize; // (in bytes - must be a multiple of 4KiB)

      enum {

        UnknownImageType = 0,
        ElfImageType = 1,
        MaxImageType

      } Type;

      struct {

        uptr Entrypoint;
        uptr Header; // (optional for RawImageType)

      } __attribute__((packed)) Executable;

    } __attribute__((packed)) Image;

    // [Memory-related information]

    struct {

      uint16 NumEntries;
      uptr List; // (pointer to a list of usableMmapEntry{})

      uint64 PreserveUntilOffset; // (tell the kernel to keep everything below this untouched)

    } __attribute__((packed)) Memory;

    // [System-related information]

    struct {

      enum : uint16 {

        UnknownArchitecture = 0,
        x64Architecture = 1

      } Architecture;

      struct {

        bool IsSupported;
        uptr Table;

      } __attribute__((packed)) Acpi;

      struct {

        struct {

          uint64 Cr0, Cr3, Cr4;
          uint64 Efer;

        } __attribute__((packed)) x64;

      } __attribute__((packed)) Cpu;

      struct {

        bool IsSupported;
        uptr Table;

      } __attribute__((packed)) Smbios;

    } __attribute__((packed)) System;

    // [Table checksum]

    uint16 Checksum; // (Sum all bytes in the table, up to Checksum)

  } __attribute__((packed)) commonInfoTable;



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
    entrypointSystemInvalidCpuData,

    // (High bit is 8h - compile or runtime issue)

    entrypointKernelNotPositionIndependent = (8ULL << 32)

  } entrypointReturnStatus;



  /* [[maybe_unused]] static const char** EntrypointStatusMessage[n]

     Calling convention: [a] -> High bits of entrypointReturnValue
                         [b] -> Low bits of entrypointReturnValue

     Format: char* (must be converted to char16* on EFI systems)

     (TODO: This is 'more' self-explanatory, but explain what it is still)
     (TODO: Provide an example)

  */

  #define GetEntrypointStatus(Value) (EntrypointStatusCodes[Value >> 32][Value & 0xFFFFFFFF])

  [[maybe_unused]] static const char** EntrypointStatusMessage[9] = {

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
      "The bootloader-provided CPU information appears to be invalid."

    },

    (const char*[]) {

      "The kernel was not compiled as a pure position-independent executable;\n\r"
      "make sure you're linking with `-pie -Wl,--no-dynamic-linker` (and not \n\r"
      "`-static-pie`)."

    }

  };

#endif
