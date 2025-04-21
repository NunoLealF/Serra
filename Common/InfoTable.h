// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_INFOTABLE_H
#define SERRA_KERNEL_INFOTABLE_H

  // Generic defines

  #define getInfoTableVersion(Major, Minor) ((Major * 0x100) + Minor)
  #define KernelInfoTableVersion getInfoTableVersion(1, 0)

  // Basically, you fill in Address in 32-bit mode, and you can use it
  // normally in 64-bit mode with the void*.

  typedef union __universalPtr {

    uint64 Address;
    void* Ptr;

  } universalPtr;

  // (BIOS table)

  typedef struct {

    // (Features, tables, etc.)

    bool AcpiSupported; // Is ACPI supported?
    universalPtr AcpiRsdp; // If so, ptr to rsdp
    universalPtr AcpiSdt; // RSDT *or* XSDT, but always the right one

    bool PciBiosSupported; // Is PCI-BIOS supported?
    universalPtr PciBiosTable;

    bool SmbiosSupported;
    universalPtr SmbiosTable;

    // (Pointers to important things)

    universalPtr Ebda;
    universalPtr RmWrapper; // Real mode wrapper from earlier

  } __attribute__((packed)) KernelBiosInfoTable;

  // (EFI table)

  typedef struct {

  } __attribute__((packed)) KernelEfiInfoTable;

  // (Main kernel info table)

  typedef enum : uint8 {

    ByUnknown = 0,
    ByDefault = 1,
    ByKeyboard = 2,
    ByFast = 3

  } a20Enum;

  typedef enum : uint8 {

    Int13 = 0,
    Int13WithEdd = 1,
    Efi = 2,
    AtaPio = 3,
    Ahci = 4,
    Nvme = 5,
    Usb = 6,
    Other = 7

  } diskAccessMethodEnum;

  typedef enum : uint8 {

    None = 0,
    Vga = 1, // (VGA text mode.)
    Vesa = 2,
    Gop = 3

  } graphicsTypeEnum;

  typedef struct {

    // (Table section)

    uint64 Signature; // Must be 7577757E7577757Eh.
    uint32 Version; // Must match KernelInfoTableVersion.
    uint32 Size; // Must match sizeof(KernelInfoTable).

    // (System section - x64-specific, of course)

    struct __System {

      a20Enum A20Method; // What method was used to enable the A20 line?

      uint32 CpuidHighestStdLevel; // Highest standard CPUID level
      uint32 CpuidHighestExtLevel; // Highest extended CPUID level

      bool PatSupported; // Is PAT supported?
      uint64 PatMsr; // The value of the PAT MSR, if it's supported.. or 0

    } __attribute__((packed)) System;

    // (System-independent sections | Memory)

    struct __Memory {

      // (System mmap is provided by BIOS or UEFI).

      uint16 NumUsableMemoryMapEntries; // Number of usable mmap entries.
      universalPtr UsableMemoryMap; // Pointer to the usable memory map

      uint64 PreserveOffset; // same as Offset, really

    } __attribute__((packed)) Memory;

    // (System-independent sections | Filesystem and disk)

    struct __FsDisk {

      diskAccessMethodEnum DiskAccessMethod; // What method *can* we use for the disk?
      uint8 DriveNumber; // If we used the BIOS, set the int13h drive number

      uint64 BpbAddress; // Address of the BPB
      uint16 PhysicalBytesPerSector; // On the boot disk
      uint16 LogicalBytesPerSector; // On the boot fs

      uint64 PartitionOffset; // Offset of the current partition, in LBA

    } __attribute__((packed)) FsDisk;

    // (System-independent sections | Kernel environment)

    struct __Kernel {

      bool Debug; // Debug flag.
      universalPtr ElfHeader; // Pointer to the kernel elf header

      uint64 UsableArea; // Start of usable mem segment (usually 80h.low)
      uint64 ModuleArea; // Start of driver/module segment (usually E0h.low)
      uint64 KernelArea; // Start of kernel segment (usually F0h.low)

    } __attribute__((packed)) Kernel;

    // (System-independent sections | Graphics)

    struct __Graphics {

      graphicsTypeEnum Type; // What type of graphics?

      struct __Vga {

        uint16 PosX;
        uint16 PosY;

        uint16 LimitX;
        uint16 LimitY;

        universalPtr Framebuffer;

      } __attribute__((packed)) Vga;

      struct __Vesa {

        universalPtr VbeInfoBlock; // VBE info block
        universalPtr VbeModeInfo; // VBE mode info block (of the current mode)
        uint16 CurrentVbeMode; // Current mode number.

        bool EdidIsSupported; // Is EDID supported?
        universalPtr EdidInfo; // EDID info block, if supported.

      } __attribute__((packed)) Vesa;

      struct __Gop {

        // (TODO: Everything)

      } __attribute__((packed)) Gop;

    } __attribute__((packed)) Graphics;

    // (Firmware and external sections)

    struct __Firmware {

      bool IsEfi; // False for BIOS, true for EFI.

      union {
        uint64 Address;
        KernelBiosInfoTable* Table;
      } BiosInfo;

      union {
        uint64 Address;
        KernelEfiInfoTable* Table;
      } EfiInfo;

    } __attribute__((packed)) Firmware;

    // (Checksum)

    uint16 Checksum;

  } __attribute__((packed)) KernelInfoTable;

#endif
