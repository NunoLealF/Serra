// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_H
#define SERRA_KERNEL_DISK_H

  // Import standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  #include "Bios/Bios.h"
  #include "Efi/Efi.h"

  // Include data structures from Disk.c

  typedef struct _diskInfo {

    // [Has this subsystem been initialized yet?]

    bool IsEnabled;

    // [Which method did we boot with - and can be guaranteed to work?]

    enum {

      BootMethod_Unknown = 0,
      BootMethod_Efi,
      BootMethod_Int13

    } BootMethod;

    // [EFI Boot Services specific information]

    struct {

      const void* Handle; // (The efiSimpleFilesystemProtocol handle we used)
      const void* Protocol; // (A pointer to the efiFileProtocol we used)

      const void* FileInfo; // (A pointer to our kernel file's efiFileInfo table)

    } __attribute__((packed)) Efi;

    // [BIOS / Int 13h specific information]

    struct {

      uint8 DriveNumber; // (The drive number we need to pass on, in DL)
      bool EddSupported; // (Are EDD extensions to int 13h supported?)

      uint16 BytesPerSector; // (The number of bytes per sector - *EDD only, or 512*)
      uint64 NumSectors; // (The total number of sectors on the drive - *EDD only*)

    } __attribute__((packed)) Int13;

  } diskInfo;

  typedef struct _volumeInfo {

    // [The method needed to access this volume, as well as the drive
    // and partition number - `method()drive()partition()`]

    enum : uint16 {

      // ('Universal' volume types that correspond to `DiskInfo.BootMethod`,
      // and that can be read from at boot-time)

      VolumeMethod_Unknown = 0, // (You can probably ignore this)
      VolumeMethod_EfiBlockIo, // (Can be accessed via efiBlockIoProtocol)
      VolumeMethod_Int13 // (Can be accessed via the int 13h wrapper)

      // (Additional volume types that can be set up later on for
      // specific types of devices - TODO)

    } Method;

    uint32 Drive; // (The drive number (implementation varies))
    uint16 Partition; // (The partition number, or 0 if raw/unpartitioned)

    // [Information about the volume's type (and partition, if one exists)]

    enum : uint16 {

      // (If this volume represents an entire device, and is partitioned,
      // show the partition map type (for example, MBR or GPT))

      // `Partition == 0`, bit 8 is cleared.

      VolumeType_Unknown = 0, // (Couldn't determine type)
      VolumeType_Mbr, // (Appears to be a 'raw' MBR volume)
      VolumeType_Gpt, // (Appears to be a 'raw' GPT volume)

      // (Otherwise, if it only represents a specific partition, or the
      // device itself is unpartitioned, show the filesystem type)

      // `Partition != 0`, bit 8 is set.

      VolumeType_Fat12 = (1ULL << 8), // (Appears to be a FAT12 partition)
      VolumeType_Fat16, // (Appears to be a FAT16 partition)
      VolumeType_Fat32, // (Appears to be a FAT32 partition)

      // TODO - Add more partition types, FAT is just the bare minimum

    } Type;

    bool IsPartition; // (Whether this volume represents a specific *partition* or not)
    uint64 PartitionOffset; // (The partition's LBA offset, *if needed*)

    // [Information about how to interact with the volume]

    uint16 Alignment; // (The alignment requirement for a transfer buffer, *as a power of 2*)
    uint32 BytesPerSector; // (How many bytes per sector/block?)
    uint32 MediaId; // (The media ID of the device (*EFI Block IO only*))
    uint64 NumSectors; // (The total number of sectors/blocks in the volume)

  } __attribute__((packed)) volumeInfo;

  // Include functions and global variables from Disk.c

  extern diskInfo DiskInfo;

  extern volumeInfo VolumeList[512];
  extern uint16 NumVolumes;

  [[nodiscard]] bool InitializeDiskSubsystem(void* InfoTable);
  bool TerminateDiskSubsystem(void);

  [[nodiscard]] bool ReadSectors(void* Buffer, uint64 Lba, uint64 NumSectors, uint16 VolumeNum);
  [[nodiscard]] bool ReadDisk(void* Buffer, uint64 Offset, uint64 Size, uint16 VolumeNum);

  // Include data structures used in Fs.c

  typedef struct _chsAddress {

    _BitInt(8) Heads : 8; // (The head value; this can be between 0 and 254~255)
    _BitInt(6) Sectors : 6; // (The sector value; this can be between 0 and 63)
    _BitInt(10) Cylinders : 10; // (The cylinder value; this can be between 0 and 1023)

  } __attribute__((packed)) chsAddress;

  static_assert((sizeof(chsAddress) == 3), "`chsAddress` was not packed correctly by the compiler.");

  typedef struct _mbrHeader {

    // [Partition entries]

    struct {

      uint8 Attributes; // (Bit 7 means the partition is 'active')
      chsAddress ChsStart; // (The starting sector, in CHS format)

      enum : uint8 {

        MbrEntryType_None = 0x00, // (Partition doesn't exist)

        MbrEntryType_Fat12 = 0x01, // (FAT12 partition, small)

        MbrEntryType_Fat16_A = 0x04, // (FAT16 partition, small)
        MbrEntryType_Fat16_B = 0x06, // (FAT16 partition, large)

        MbrEntryType_Fat32_A = 0x0B, // (FAT32 partition, CHS variant)
        MbrEntryType_Fat32_B = 0x0C, // (FAT32 partition, LBA variant)
        MbrEntryType_Fat16_C = 0x0E, // (FAT16 partition, LBA)

        MbrEntryType_Gpt = 0xEE, // (Protective MBR partition)
        MbrEntryType_Esp = 0xEF, // (EFI System Partition (FAT))

      } Type;

      chsAddress ChsEnd; // (The *last* sector, in CHS format)

      uint32 Lba; // (LBA start address)
      uint32 NumSectors; // (Number of sectors)

    } __attribute__((packed)) Entry[4];

    // [Boot signature - this *must* match AA55h]

    uint16 Signature;

  } __attribute__((packed)) mbrHeader;

  static_assert((sizeof(mbrHeader) == 66), "`mbrHeader` was not packed correctly by the compiler.");

  typedef struct _gptHeader {



  } __attribute__((packed)) gptHeader;

  // Include functions and global variables from Fs.c

  [[nodiscard]] bool InitializeFsSubsystem(void);

#endif
