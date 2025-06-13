// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_FS_H
#define SERRA_KERNEL_DISK_FS_H

  // Include CRC-related functions from Crc32.c

  uint32 CalculateCrc32(void* Buffer, uintptr Length);

  // Include data structures from Fs.c

  typedef struct _chsAddress {

    uint16 Heads : 8; // (The head value; this can be between 0 and 254~255)
    uint16 Sectors : 6; // (The sector value; this can be between 0 and 63)
    uint16 Cylinders : 10; // (The cylinder value; this can be between 0 and 1023)

  } __attribute__((packed)) chsAddress;

  #define mbrHeaderSignature 0xAA55

  typedef struct _mbrHeader {

    // [Bootstrap code, and optional attributes that we don't use]

    uint8 Bootstrap[440]; // (Bootstrap code - we don't use this at all)
    uint8 Optional[6]; // (Optional attributs - we don't use these either)

    // [Partition entries]

    struct {

      uint8 Attributes; // (Bit 7 means the partition is 'active')
      chsAddress ChsStart; // (The starting sector, in CHS format)

      enum : uint8 {

        MbrPartitionType_None = 0x00, // (Partition doesn't exist)

        MbrPartitionType_Fat12 = 0x01, // (FAT12 partition, small)

        MbrPartitionType_Fat16_A = 0x04, // (FAT16 partition, small)
        MbrPartitionType_Fat16_B = 0x06, // (FAT16 partition, large)

        MbrPartitionType_Exfat = 0x07, // (exFAT or NTFS partition)
        MbrPartitionType_Ntfs = 0x07, // (exFAT or NTFS partition)

        MbrPartitionType_Fat32_A = 0x0B, // (FAT32 partition, CHS variant)
        MbrPartitionType_Fat32_B = 0x0C, // (FAT32 partition, LBA variant)
        MbrPartitionType_Fat16_C = 0x0E, // (FAT16 partition, LBA)

        MbrPartitionType_Gpt = 0xEE, // (Protective MBR partition)
        MbrPartitionType_Esp = 0xEF // (EFI System Partition (FAT))

      } Type;

      chsAddress ChsEnd; // (The *last* sector, in CHS format)

      uint32 Lba; // (LBA start address)
      uint32 NumSectors; // (Number of sectors)

    } __attribute__((packed)) Entry[4];

    // [Bootsector signature]

    uint16 Signature; // (This *must* match `mbrHeaderSignature`, or AA55h)

  } __attribute__((packed)) mbrHeader;

  #define gptHeaderSignature 0x5452415020494645
  #define gptHeaderRevision 0x10000

  typedef struct _gptHeader {

    // [Header-related information]

    uint64 Signature; // (Must match `gptHeaderSignature`, or 5452415020494645h)
    uint32 Revision; // (Should be at least 10000h (EFI 1.1))
    uint32 Size; // (Should be at least sizeof(gptHeader))

    uint32 Crc32; // (The CRC-32 hash of the table, with this field zeroed out)
    uint32 Reserved; // (Reserved for firmware use, apparently?)

    // [Partition-related information]

    uint64 HeaderLba; // (The LBA of the sector containing this data structure)
    uint64 BackupLba; // (Same as above, but for the backup header)

    uint64 FirstUsableLba; // (The *first* usable sector on the disk)
    uint64 LastUsableLba; // (The *last* usable sector on the disk)

    genericUuid DiskUuid; // (The UUID/GUID of this specific disk)

    // [Partition-entry-related information]

    uint64 PartitionLba; // (The first sector that holds partition entries)
    uint32 NumPartitions; // (The number of partition entries)
    uint32 PartitionEntrySize; // (The size of a partition entry)
    uint32 PartitionCrc32; // (The CRC-32 hash of every partition entry)

  } __attribute__((packed)) gptHeader;

  constexpr genericUuid GptPartitionType_None = // (Partition doesn't exist)
  {0x00000000, {0x0000, 0x0000}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

  constexpr genericUuid GptPartitionType_BasicData = // (Microsoft basic data partition - can be FAT, exFAT or NTFS)
  {0xEBD0A0A2, {0xB9E5, 0x4433}, {0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};

  constexpr genericUuid GptPartitionType_Esp = // (EFI System Partition - can be FAT)
  {0xC12A7328, {0xF81F, 0x11D2}, {0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

  constexpr genericUuid GptPartitionType_LinuxData = // (Linux basic data partition - can be anything)
  {0x4F68BCE3, {0xE8CD, 0x4DB1}, {0x96, 0xE7, 0xFB, 0xCA, 0xF9, 0x84, 0xB7, 0x09}};

  typedef struct _gptPartition {

    genericUuid Type; // (The partition type - see GptPartitionType_*)
    genericUuid UniqueId; // (The unique UUID of this specific partition)

    uint64 StartingLba; // (The *first* LBA that belongs to this partition)
    uint64 EndingLba; // (The *last* LBA that belongs to this partition)

    uint64 Attributes; // (The partition's attributes)
    char16 Name[36]; // (The partition name - this uses 16-bit characters)

  } __attribute__((packed)) gptPartition;

  static_assert((sizeof(chsAddress) == 3), "chsAddress{} was not packed correctly by the compiler.");
  static_assert((sizeof(mbrHeader) == 512), "mbrHeader{} was not packed correctly by the compiler.");
  static_assert((sizeof(gptHeader) == 92), "gptHeader{} was not packed correctly by the compiler.");
  static_assert((sizeof(gptPartition) == 128), "gptPartition{} was not packed correctly by the compiler.");

  // Include functions and global variables from Fs.c

  [[nodiscard]] bool InitializeFsSubsystem(void);

#endif
