// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_INFOTABLES_H
#define SERRA_INFOTABLES_H

  // This table will be passed from the second-stage bootloader onto the third-stage
  // bootloader; it's supposed to be placed at AE00h, and because of that, it has a maximum size
  // of 512 bytes.

  // [VERSION 1; SIZE = 74 BYTES]

  #define InfoTable_Location 0xAE00

  typedef struct {

    // [Table-related info]

    uint32 Signature; // This is basically a magic number; set it to 65363231h
    uint16 Version; // For now, 1

    uint16 Size; // Maximum 512 bytes

    // [System-related]

    struct __System_Info {

      bool Debug;

    } __attribute__((packed)) System_Info;

    // [Disk/EDD]

    uint8 DriveNumber; // Same as SaveDL, or if Edd_Valid is false, 0x80
    bool Edd_Valid; // Whether EDD functions work or not

    uint16 LogicalSectorSize; // FAT/BPB
    uint16 PhysicalSectorSize; // EDD/int13h

    struct __Edd_Info {

      // [EDD 1.x]

      uint16 Size;
      uint16 Flags;

      uint32 NumPhysicalCylinders;
      uint32 NumPhysicalHeads;
      uint32 NumPhysicalSectors;

      uint64 NumSectors;
      uint16 BytesPerSector;

      // [EDD 2.x and 3.x]

      uint8 Edd_ExtraData[40];

    } __attribute__((packed)) Edd_Info;

    // [Filesystem and BPB]

    bool Bpb_IsFat32;
    uint8 Bpb[120]; // Regular+extended BPB (usually starts at 7C00h + 3!)

    // [Terminal]

    struct __Terminal_Info {

      uint16 PosX;
      uint16 PosY;
      uint32 Framebuffer;

      uint16 LimitX;
      uint16 LimitY;

    } __attribute__((packed)) Terminal_Info;

  } __attribute__((packed)) bootloaderInfoTable;

#endif
