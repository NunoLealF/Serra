// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_COMMON_INFOTABLE_H
#define SERRA_COMMON_INFOTABLE_H

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
  #define commonInfoTableVersion 2

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
        Int13Method = 2

      } AccessMethod;

      struct {

        uptr FsHandle;
        uptr FsProtocolHandle;

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
        GopDisplay = 4

      } Type;

      struct {

        bool IsSupported;
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

          AsciiFormat = 0,
          Utf16Format = 1

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
        EfiFirmware = 2

      } Type;

      struct {

        enum : uint8 {

          EnabledByUnknown = 0,
          EnabledByDefault = 1,
          EnabledByKeyboard = 2,
          EnabledByFast = 3

        } A20;

        struct {

          uint16 NumEntries;
          uint8 EntrySize;
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

        bool SupportsConIn;
        bool SupportsConOut;

        struct {

          bool IsSupported;
          uptr Protocol;

        } __attribute__((packed)) Gop;

        struct {

          uint16 NumEntries;
          uint8 EntrySize;
          uptr List; // (pointer to a list of efiMmapEntry{})

        } __attribute__((packed)) Mmap;

        struct {

          uptr BootServices;
          uptr SystemTable;
          uptr RuntimeServices;

        } __attribute__((packed)) Tables;

      } Efi;

    } __attribute__((packed)) Firmware;

    // [Image-related information]

    struct {

      uptr Stack;

      enum {

        RawImageType = 0,
        ElfImageType = 1,
        PeImageType = 2

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

      uint64 PreserveUntilOffset; // Don't touch *anything* until..

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

          uint64 Cr0, Cr3, Cr4; // (optional if ProtectionLevel != 0)
          uint64 Efer; // (optional if ProtectionLevel != 0)
          uint64 Pml4; // (optional if ProtectionLevel != 0)

          uint8 ProtectionLevel;

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















  // ====================================================================
  //                [EVERYTHING BELOW THIS IS DEPRECATED]




  // Generic defines

  #define getInfoTableVersion(Major, Minor) ((Major * 0x100) + Minor)
  #define KernelInfoTableVersion getInfoTableVersion(0, 1)



  // Basically, you fill in Address in 32-bit mode, and you can use it
  // normally in 64-bit mode with the void*.

  typedef union _universalPtr {

    uint64 Address;
    void* Ptr;

  } universalPtr;



  // (BIOS table)

  typedef struct _kernelBiosInfoTable {

    // (Pointers to BIOS-specific stuff)

    bool PciBiosSupported; // Is PCI-BIOS supported?
    universalPtr PciBiosTable;

    // (Pointers to important things)

    universalPtr RmWrapper; // Real mode wrapper from earlier

  } __attribute__((packed)) kernelBiosInfoTable;



  // (EFI table)

  typedef struct _kernelEfiInfoTable {

    // (Pointer to EFI image handle)

    universalPtr ImageHandle;

    // (Pointers to tables)

    universalPtr SystemTable;
    universalPtr BootServices;
    universalPtr RuntimeServices;

  } __attribute__((packed)) kernelEfiInfoTable;



  // (Main kernel info table)

  typedef enum _a20Enum : uint8 {

    ByUnknown = 0,
    ByDefault = 1,
    ByKeyboard = 2,
    ByFast = 3

  } a20Enum;

  typedef enum _diskAccessMethodEnum : uint8 {

    Int13 = 0,
    Int13WithEdd = 1,
    Efi = 2,
    AtaPio = 3,
    Ahci = 4,
    Nvme = 5,
    Usb = 6,
    Other = 7

  } diskAccessMethodEnum;

  typedef enum _graphicsTypeEnum : uint8 {

    None = 0, // None known
    VgaText = 1, // VGA text mode
    EfiText = 2, // EFI text output protocol
    Vesa = 3,  // VESA graphics mode
    Gop = 4 // EFI graphics output protocol

  } graphicsTypeEnum;

  typedef enum _mmapTypeEnum : uint8 {

    Unknown = 0, // Unknown
    BiosMmap = 1, // (int 15h / eax E820h), normal for BIOS
    EfiMmap = 2 // From GetMemoryMap()

  } mmapTypeEnum;



  typedef struct _kernelInfoTable {

    // (Table section)

    uint64 Signature; // Must be 7577757E7577757Eh.
    uint32 Version; // Must match KernelInfoTableVersion.
    uint32 Size; // Must match sizeof(KernelInfoTable).

    // (System section - x64-specific, of course)

    struct System2 {

      a20Enum A20Method; // What method was used to enable the A20 line?

      bool AcpiSupported; // Is ACPI supported?
      universalPtr AcpiRsdp; // If so, pointer to RSDP

      uint8 Cpl; // What CPU protection level are we in? (Ring 0 only for BIOS, any ring for UEFI though with limitations)

      uint64 Cr0; // Value of the CR0 control register, if supported
      uint64 Cr3; // Value of the CR3 control register, if supported
      uint64 Cr4; // Value of the CR4 control register, if supported
      uint64 Efer; // Value of the EFER model-specific register, if supported

      bool PatSupported; // Is PAT supported? (BIOS-only)
      uint64 PatMsr; // The value of the PAT MSR, if supported (BIOS-only)

      universalPtr Pml4; // Location of the PML4

      bool SmbiosSupported; // Is SMBIOS supported?
      universalPtr SmbiosTable; // If so, here's a pointer

    } __attribute__((packed)) System;

    // (System-independent sections | Memory)

    struct Memory2 {

      mmapTypeEnum Type; // What type of mmap are we dealing with here?

      // (System mmap - format can vary depending on BIOS or UEFI.)

      uint16 NumMmapEntries;
      universalPtr MemoryMap;
      uint8 MemoryMapEntrySize;

      // (Usable mmap - format must be kernelUsableMmap{})

      uint16 NumUsableMmapEntries; // Number of usable mmap entries.
      universalPtr UsableMemoryMap; // Pointer to the usable memory map

      uint64 PreserveOffset; // same as Offset, really

    } __attribute__((packed)) Memory;

    // (System-independent sections | Filesystem and disk)

    struct FsDisk2 {

      diskAccessMethodEnum DiskAccessMethod; // What method *can* we use for the disk?
      uint8 DriveNumber; // If we used the BIOS, set the int13h drive number

      universalPtr Bpb; // Address of the BPB
      uint16 PhysicalBytesPerSector; // On the boot disk
      uint16 LogicalBytesPerSector; // On the boot fs

      uint64 PartitionOffset; // Offset of the current partition, in LBA

    } __attribute__((packed)) FsDisk;

    // (System-independent sections | Kernel environment)

    struct Kernel2 {

      universalPtr ElfHeader; // Pointer to the kernel ELF header
      universalPtr Entrypoint; // Kernel entrypoint *and* initial stack ptr
      universalPtr Stack; // Initial stack pointer (minus 128 bytes, as in stub)

    } __attribute__((packed)) Kernel;

    // (System-independent sections | Graphics)

    struct Graphics2 {

      graphicsTypeEnum Type; // What type of graphics?

      struct VgaText2 {

        uint16 PosX;
        uint16 PosY;

        uint16 LimitX;
        uint16 LimitY;

        uint32 Framebuffer;

      } __attribute__((packed)) VgaText;

      struct Vesa2 {

        universalPtr VbeInfoBlock; // VBE info block
        universalPtr VbeModeInfo; // VBE mode info block (of the current mode)
        uint16 CurrentVbeMode; // Current mode number.

        uint64 Framebuffer; // ...

        bool EdidIsSupported; // Is EDID supported?
        universalPtr EdidInfo; // EDID info block, if supported.

      } __attribute__((packed)) Vesa;

      struct EfiText2 {

        // (TODO: everything)
        char PLACEHOLDER;

      } __attribute__((packed)) EfiText;

      struct Gop2 {

        // (TODO: everything)
        char PLACEHOLDER;

      } __attribute__((packed)) Gop;

    } __attribute__((packed)) Graphics;

    // (Firmware and external sections)

    struct Firmware2 {

      bool IsEfi; // False for BIOS, true for EFI.

      union {
        uint64 Address;
        kernelBiosInfoTable* Table;
      } BiosInfo;

      union {
        uint64 Address;
        kernelEfiInfoTable* Table;
      } EfiInfo;

    } __attribute__((packed)) Firmware;

    // (Checksum)

    uint16 Checksum;

  } __attribute__((packed)) kernelInfoTable;

#endif
