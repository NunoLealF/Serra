// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_INFOTABLES_H
#define SERRA_INFOTABLES_H

  // This table will be passed from the second-stage bootloader onto the third-stage
  // bootloader; it's supposed to be placed at AE00h, and because of that, it has a maximum size
  // of 512 bytes.

  // (right now, this has about 82 bytes, so it's not that huge)

  typedef struct {

    // [Start of table]

    uint32 Signature; // This is basically a magic number; set it to 65363231h
    uint16 Version; // For now, 0001h

    uint16 TableSize; // Maximum 512 bytes

    // [System-related; A20 info]

    bool A20_EnabledByDefault;
    bool A20_EnabledByKbd;
    bool A20_EnabledByFast;

    // [System-related; other info]

    bool Debug;

    // [Disk-related; sector size]

    uint16 LogicalSectorSize; // FAT/BPB
    uint16 PhysicalSectorSize; // EDD/int13h

    // [Disk-related; drive number and EDD]

    uint8 Edd_DriveNumber; // Same as SaveDL, or if Edd_Valid is false, 0x80
    bool Edd_Valid; // Whether EDD functions work or not

    uint64 Edd_NumSectors; // (Only valid if EDD_Valid == true)

    uint32 Edd_NumPhysicalCylinders; // (Only valid if EDD_Valid == true)
    uint32 Edd_NumPhysicalHeads; // (Only valid if EDD_Valid == true)
    uint32 Edd_NumPhysicalSectors; // (Only valid if EDD_Valid == true)

    uint64 Edd_NumSectors; // (Only valid if EDD_Valid == true)

    // [Filesystem-related; basic info about the FAT partition]

    uint32 Fat_BpbLocation; // The flat address of the BPB (usually 7C00h + 3)
    bool PartitionIsFat32; // Calculated manually

    uint32 Fat_NumSectors; // Regular BPB or FAT32 extended BPB
    uint32 Fat_NumRootSectors; // Calculated manually
    uint32 Fat_NumDataSectors; // Calculated manually

    uint32 Fat_HiddenSectors; // Regular BPB
    uint16 Fat_ReservedSectors; // Regular BPB

    // [Terminal-related; other]

    bool Terminal_Usable; // Is the terminal usable or not?

    // [Terminal-related; position, framebuffer, and mode info]

    uint16 Terminal_PosX; // (Only valid if Terminal_Usable == true)
    uint16 Terminal_PosY; // (Only valid if Terminal_Usable == true)
    uint32 Terminal_Framebuffer; // (Only valid if Terminal_Usable == true)

    uint16 Terminal_LimitX; // (Only valid if Terminal_Usable == true)
    uint16 Terminal_LimitY; // (Only valid if Terminal_Usable == true)

  } __attribute__((packed)) bootloaderInfoTable;

#endif
