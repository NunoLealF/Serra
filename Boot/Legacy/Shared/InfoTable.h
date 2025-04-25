// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOT_INFOTABLE_H
#define SERRA_BOOT_INFOTABLE_H

  // This table will be passed from the second-stage bootloader onto the third-stage
  // bootloader; it's supposed to be placed at AE00h, and because of that, it has a maximum size
  // of 512 bytes.

  // [VERSION 1]

  #define InfoTableLocation 0xAE00

  typedef struct __bootloaderInfoTable {

    // [Table-related info]

    uint32 Signature; // This is basically a magic number; set it to 65363231h
    uint16 Version; // For now, 1

    uint16 Size; // Maximum 512 bytes

    // [System-related]

    struct __SystemInfo {

      bool Debug;

    } __attribute__((packed)) SystemInfo;

    // [Disk/EDD]

    uint8 DriveNumber; // Same as SaveDL, or if Edd_Valid is false, 0x80
    bool EddEnabled; // Whether EDD functions work or not

    uint16 LogicalSectorSize; // FAT/BPB
    uint16 PhysicalSectorSize; // EDD/int13h

    struct __EddInfo {

      uint16 Size;
      uint16 Flags;

      uint32 NumPhysicalCylinders;
      uint32 NumPhysicalHeads;
      uint32 NumPhysicalSectors;

      uint64 NumSectors;
      uint16 BytesPerSector;

    } __attribute__((packed)) EddInfo;

    // [Filesystem and BPB]

    bool BpbIsFat32;
    uint8 Bpb[120]; // Regular+extended BPB (usually starts at 7C00h + 3!)

    // [Terminal]

    struct __TerminalInfo {

      uint16 PosX;
      uint16 PosY;
      uint32 Framebuffer;

      uint16 LimitX;
      uint16 LimitY;

    } __attribute__((packed)) TerminalInfo;

  } __attribute__((packed)) bootloaderInfoTable;

#endif
