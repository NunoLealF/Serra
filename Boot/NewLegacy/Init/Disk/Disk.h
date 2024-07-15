// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_DISK_H
#define SERRA_DISK_H

  // (int 13h, ah=48h)
  // https://www.ctyme.com/intr/rb-0715.htm#Table274

  // (We'll only actually work with / implement EDD 1.x, but just in case, we'll also reserve
  // space for EDD 2.x / EDD 3.x)

  typedef volatile struct {

    // [EDD 1.x, safe to use in any version]

    uint16 Size; // Call size of the buffer. (ALSO, SET THIS TO AT LEAST 42H FIRST!!)
    uint16 Flags; // Flags, see #0274

    uint32 NumPhysicalCylinders; // Number of physical cylinders
    uint32 NumPhysicalHeads; // Number of physical heads
    uint32 NumPhysicalSectors; // Number of physical sectors *per track*

    uint64 NumSectors; // Total number of sectors on the drive
    uint16 BytesPerSector; // Number of bytes per sector (can be 512, 2048, etc.!)

    // [EDD 2.x, not safe for versions below]

    uint8 Edd2_Data[4]; // Table #00278, or just FFFFFFFF if not available (?)

    // [EDD 3.x, not safe for versions below]

    uint8 Edd3_Data[36]; // ...

  } __attribute__((packed)) eddDriveParameters;

  // An EDD disk address packet

  typedef volatile struct {

    uint8 Size;
    uint8 Reserved;

    uint16 NumBlocks;
    uint32 Address;
    uint64 Lba;

  } __attribute__((packed)) eddDiskAddressPacket;



  // The BIOS parameter block; the first part is universal for FAT16/FAT32, then there's
  // different structures depending on whether it's FAT16 or FAT32.

  typedef volatile struct {

    uint8 Identifier[8]; // OEM identifier

    uint16 BytesPerSector; // How many bytes per sector? (MAY BE UNRELIABLE, DON'T TRUST UNLESS NECESSARY)
    uint8 SectorsPerCluster; // How many sectors are in a FAT cluster?
    uint16 ReservedSectors; // How many sectors are reserved *from the start of this partition*?

    uint8 NumFileAllocationTables; // How many FATs are on this partition?
    uint16 NumRootEntries; // How many root directory entries are on this partition?

    uint16 NumSectors; // The number of sectors on this partition (FAT16 ONLY)
    uint8 MediaDescriptorType; // The media descriptor type of the drive with this partition.
    uint16 SectorsPerFat; // The number of sectors per FAT (FAT16 ONLY)
    uint16 SectorsPerTrack; // The number of sectors per track (MAY BE UNRELIABLE, DON'T TRUST UNLESS NECESSARY)
    uint16 NumPhysicalHeads; // The number of heads on the drive with this partition (MAY BE UNRELIABLE, DON'T TRUST UNLESS NECESSARY)

    uint32 HiddenSectors; // The number of hidden sectors - *in practice, this is the starting LBA of this partition*
    uint32 NumSectors_Large; // The number of sectors on this partition, but for sector counts above 2^16 (FAT32 ONLY)

  } __attribute__((packed)) biosParameterBlock;



  // (extended BPB / extended boot record, for FAT16)

  typedef volatile struct {

    uint8 DriveNumber; // The drive number of the current media/drive (VERY UNRELIABLE, DON'T TRUST UNLESS NECESSARY)
    uint8 NtFlags; // Flags, reserved for use by some operating systems
    uint8 Signature; // The signature; this has to be either 28h or 29h for some systems to boot.

    uint8 VolumeSerial[4]; // The 'serial number' of this volume ID, this can basically be ignored.
    uint8 VolumeID[11]; // The actual ID of the volume in question (padded with spaces, NOT nulls)
    uint8 Identifier[8]; // The system identifier string, also padded with spaces (TODO: is it the same as the first entry in the regular bpb (identifier[8])??)

  } __attribute__((packed)) biosParameterBlock_Fat16;



  // (extended BPB / extended boot record, for FAT32)

  typedef volatile struct {

    uint32 SectorsPerFat; // The number of sectors per FAT (replaces the FAT16 entry in the BPB)
    uint16 Flags; // Just.. flags (I need a proper copy of the FAT32 spec)
    uint16 Version; // The FAT32 version of this partition

    uint32 RootCluster; // The cluster of the root directory
    uint16 FsInfoSector; // The sector number of the FSInfo structure
    uint16 BackupSector; // The sector number of the backup bootsector/VBR

    uint8 Reserved[12]; // Reserved for later..

    uint8 DriveNumber; // The drive number of the current media/drive (VERY UNRELIABLE, DON'T TRUST UNLESS NECESSARY)
    uint8 NtFlags; // Flags, reserved for use by some operating systems
    uint8 Signature; // The signature; this has to be either 28h or 29h for some systems to boot.

    uint8 VolumeSerial[4]; // The 'serial number' of this volume ID, this can basically be ignored.
    uint8 VolumeID[11]; // The actual ID of the volume in question (padded with spaces, NOT nulls)
    uint8 Identifier[8]; // The identifier string; in this case, it's actually "FAT32   " (again, padded with spaces).

  } __attribute__((packed)) biosParameterBlock_Fat32;



#endif
