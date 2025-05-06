// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_INFOTABLE_H
#define SERRA_KERNEL_INFOTABLE_H

  // Generic defines

  #define getInfoTableVersion(Major, Minor) ((Major * 0x100) + Minor)
  #define KernelInfoTableVersion getInfoTableVersion(0, 1)



  // Basically, you fill in Address in 32-bit mode, and you can use it
  // normally in 64-bit mode with the void*.

  typedef union __universalPtr {

    uint64 Address;
    void* Ptr;

  } universalPtr;

  // Usable memory map format

  typedef struct __usableMmapEntry {

    uint64 Base;
    uint64 Limit;

  } __attribute__((packed)) usableMmapEntry;



  // (BIOS table)

  typedef struct __kernelBiosInfoTable {

    // (Pointers to BIOS-specific stuff)

    bool PciBiosSupported; // Is PCI-BIOS supported?
    universalPtr PciBiosTable;

    // (Pointers to important things)

    universalPtr RmWrapper; // Real mode wrapper from earlier

  } __attribute__((packed)) kernelBiosInfoTable;



  // (EFI table)

  typedef struct __kernelEfiInfoTable {

    // (Pointer to EFI image handle)

    universalPtr ImageHandle;

    // (Pointers to tables)

    universalPtr SystemTable;
    universalPtr BootServices;
    universalPtr RuntimeServices;

  } __attribute__((packed)) kernelEfiInfoTable;



  // (Main kernel info table)

  typedef enum __a20Enum : uint8 {

    ByUnknown = 0,
    ByDefault = 1,
    ByKeyboard = 2,
    ByFast = 3

  } a20Enum;

  typedef enum __diskAccessMethodEnum : uint8 {

    Int13 = 0,
    Int13WithEdd = 1,
    Efi = 2,
    AtaPio = 3,
    Ahci = 4,
    Nvme = 5,
    Usb = 6,
    Other = 7

  } diskAccessMethodEnum;

  typedef enum __graphicsTypeEnum : uint8 {

    None = 0, // None known
    VgaText = 1, // VGA text mode
    EfiText = 2, // EFI text output protocol
    Vesa = 3,  // VESA graphics mode
    Gop = 4 // EFI graphics output protocol

  } graphicsTypeEnum;

  typedef enum __mmapTypeEnum : uint8 {

    Unknown = 0, // Unknown
    BiosMmap = 1, // (int 15h / eax E820h), normal for BIOS
    EfiMmap = 2 // From GetMemoryMap()

  } mmapTypeEnum;



  typedef struct __kernelInfoTable {

    // (Table section)

    uint64 Signature; // Must be 7577757E7577757Eh.
    uint32 Version; // Must match KernelInfoTableVersion.
    uint32 Size; // Must match sizeof(KernelInfoTable).

    // (System section - x64-specific, of course)

    struct __System {

      a20Enum A20Method; // What method was used to enable the A20 line?

      bool AcpiSupported; // Is ACPI supported?
      universalPtr AcpiRsdp; // If so, pointer to RSDP

      uint64 Cr0; // Value of the CR0 control register
      uint64 Cr3; // Value of the CR3 control register
      uint64 Cr4; // Value of the CR4 control register
      uint64 Efer; // Value of the EFER model-specific register (MSR)

      uint32 CpuidHighestStdLevel; // Highest standard CPUID level
      uint32 CpuidHighestExtLevel; // Highest extended CPUID level

      bool PatSupported; // Is PAT supported?
      uint64 PatMsr; // The value of the PAT MSR, if it's supported.. or 0

      universalPtr Pml4; // Location of the PML4

      bool SmbiosSupported; // Is SMBIOS supported?
      universalPtr SmbiosTable; // If so, here's a pointer

    } __attribute__((packed)) System;

    // (System-independent sections | Memory)

    struct __Memory {

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

    struct __FsDisk {

      diskAccessMethodEnum DiskAccessMethod; // What method *can* we use for the disk?
      uint8 DriveNumber; // If we used the BIOS, set the int13h drive number

      universalPtr Bpb; // Address of the BPB
      uint16 PhysicalBytesPerSector; // On the boot disk
      uint16 LogicalBytesPerSector; // On the boot fs

      uint64 PartitionOffset; // Offset of the current partition, in LBA

    } __attribute__((packed)) FsDisk;

    // (System-independent sections | Kernel environment)

    struct __Kernel {

      universalPtr ElfHeader; // Pointer to the kernel ELF header
      universalPtr Entrypoint; // Kernel entrypoint *and* initial stack ptr
      universalPtr Stack; // Initial stack pointer (minus 128 bytes, as in stub)

    } __attribute__((packed)) Kernel;

    // (System-independent sections | Graphics)

    struct __Graphics {

      graphicsTypeEnum Type; // What type of graphics?

      struct __VgaText {

        uint16 PosX;
        uint16 PosY;

        uint16 LimitX;
        uint16 LimitY;

        uint32 Framebuffer;

      } __attribute__((packed)) VgaText;

      struct __Vesa {

        universalPtr VbeInfoBlock; // VBE info block
        universalPtr VbeModeInfo; // VBE mode info block (of the current mode)
        uint16 CurrentVbeMode; // Current mode number.

        uint64 Framebuffer; // ...

        bool EdidIsSupported; // Is EDID supported?
        universalPtr EdidInfo; // EDID info block, if supported.

      } __attribute__((packed)) Vesa;

      struct __EfiText {

        // (TODO: everything)
        char PLACEHOLDER;

      } __attribute__((packed)) EfiText;

      struct __Gop {

        // (TODO: everything)
        char PLACEHOLDER;

      } __attribute__((packed)) Gop;

    } __attribute__((packed)) Graphics;

    // (Firmware and external sections)

    struct __Firmware {

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
