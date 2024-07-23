// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_INFOTABLES_H
#define SERRA_INFOTABLES_H

  // This table will be passed from the second-stage bootloader onto the third-stage
  // bootloader; it's supposed to be placed at AE00h, and because of that, it has a maximum size
  // of 512 bytes.

  // [VERSION 1; SIZE = 74 BYTES]

  typedef struct {

    // [Table-related info]

    uint32 Signature; // This is basically a magic number; set it to 65363231h
    uint16 Version; // For now, 1

    uint16 Size; // Maximum 512 bytes

    // [System-related]

    bool Debug;

    struct __System_Info {

      bool A20_EnabledByDefault;
      bool A20_EnabledByKbd;
      bool A20_EnabledByFast;

    } __attribute__((packed)) System_Info;

    // [Disk/EDD]

    uint8 DriveNumber; // Same as SaveDL, or if Edd_Valid is false, 0x80
    bool Edd_Valid; // Whether EDD functions work or not

    uint16 LogicalSectorSize; // FAT/BPB
    uint16 PhysicalSectorSize; // EDD/int13h

    struct __Edd_Info {

      uint16 Flags; // (Information flags)

      uint32 NumPhysicalCylinders; // (The number of physical cylinders on the drive)
      uint32 NumPhysicalHeads; // (The number of physical heads on the drive)
      uint32 NumPhysicalSectors; // (The number of physical sectors on the drive)

      uint64 NumSectors; // (The total number of sectors on the entire drive)

    } __attribute__((packed)) Edd_Info;

    // [Filesystem and BPB]

    uint32 Bpb_Location; // The flat address of the BPB (usually 7C00h + 3)
    bool PartitionIsFat32; // Calculated manually

    struct __Fat_Info {

      uint32 NumSectors; // Regular BPB or FAT32 extended BPB
      uint32 NumRootSectors; // Calculated manually
      uint32 NumDataSectors; // Calculated manually

      uint32 HiddenSectors; // Regular BPB
      uint16 ReservedSectors; // Regular BPB

    } __attribute__((packed)) Fat_Info;

    // [Terminal]

    bool TerminalIsUsable; // Is the terminal usable or not?

    struct __Terminal_Info {

      uint16 PosX;
      uint16 PosY;
      uint32 Framebuffer;

      uint16 LimitX;
      uint16 LimitY;

    } __attribute__((packed)) Terminal_Info;

  } __attribute__((packed)) bootloaderInfoTable;

#endif
