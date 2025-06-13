// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_H
#define SERRA_KERNEL_DISK_H

  // Import standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  #include "Bios/Bios.h"
  #include "Efi/Efi.h"
  #include "Fs/Fs.h"

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

      VolumeType_Unknown = 0, // (Couldn't determine type)
      VolumeType_Mbr, // (Appears to be a MBR-partitioned volume)
      VolumeType_Gpt, // (Appears to be a GPT-partitioned volume)

      // (Otherwise, if it only represents a specific partition, or the
      // device itself is unpartitioned, show the filesystem type)

      VolumeType_Partition_Unknown = (1ULL << 8), // (Couldn't determine type)
      VolumeType_Partition_BasicData, // (Analyze the filesystem type first)
      VolumeType_Partition_Fat, // (Appears to be a FAT partition)

      // TODO - Add more partition types, FAT is just the bare minimum

    } Type;

    bool IsPartition; // (Whether this volume represents a specific *partition* or not)
    uint64 PartitionOffset; // (The partition's LBA offset, *if needed*)

    // [Information about how to interact with the volume]

    uint16 Alignment; // (The alignment requirement for a transfer buffer, *as a power of 2*)
    uint32 BytesPerSector; // (How many bytes per sector/block?)
    uint32 MediaId; // (The media ID of the device (*EFI Block IO only*))
    uint64 NumSectors; // (The total number of sectors/blocks in the volume)

  } volumeInfo;

  // Include functions and global variables from Disk.c

  extern diskInfo DiskInfo;

  extern volumeInfo VolumeList[512];
  extern uint16 NumVolumes;

  [[nodiscard]] bool InitializeDiskSubsystem(void* InfoTable);
  bool TerminateDiskSubsystem(void);

  [[nodiscard]] bool ReadSectors(void* Buffer, uint64 Lba, uint64 NumSectors, uint16 VolumeNum);
  [[nodiscard]] bool ReadDisk(void* Buffer, uint64 Offset, uint64 Size, uint16 VolumeNum);

#endif
