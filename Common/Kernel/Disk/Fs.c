// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Memory/Memory.h"
#include "Disk.h"

// (DEBUG)
#include "../Libraries/Stdio.h"



// (TODO - Include a function to process an unpartitioned volume / an
// individual filesystem)



// (TODO - Include a function to validate an MBR header)

[[nodiscard]] static bool ValidateMbrHeader(mbrHeader* Header, uint16 VolumeNum) {

  // (Check whether `Header` is a valid pointer, and that `VolumeNum`
  // doesn't exceed limits)

  if (Header == NULL) {
    return false;
  } else if (VolumeNum >= NumVolumes) {
    return false;
  }

  // Before we do anything else, we need to check whether the signature
  // matches `mbrHeaderSignature` (indicating that it's a bootable drive),
  // and whether the given volume represents a partition or not.

  volumeInfo* Volume = &VolumeList[VolumeNum];

  if (Header->Signature != mbrHeaderSignature) {
    return false;
  }

  if (Volume->IsPartition == true) {
    return false;
  }

  // Now that we know we're probably looking at a valid bootsector, we
  // can run a few checks in order to verify whether it contains a
  // valid MBR or not.

  // (1) Calculate the start and end of each partition, in bytes - we
  // check the LBA values if possible, and CHS values (assuming
  // 512-byte sectors) as a fallback.

  uint64 PartitionStart[4] = {0};
  uint64 PartitionEnd[4] = {0};

  for (auto Index = 0; Index < 4; Index++) {

    // (If this entry doesn't represent any partition type, skip it)

    if (Header->Entry[Index].Type == MbrEntryType_None) {
      continue;
    }

    // (Check whether the LBA values for this specific entry are empty,
    // and if they are, use the CHS values)

    if (Header->Entry[Index].Lba != 0) {

      // (Calculate LBA values)

      // Here, we're assuming the sector size is the native one.

      if (Header->Entry[Index].NumSectors != 0) {

        PartitionStart[Index] = Header->Entry[Index].Lba;
        PartitionEnd[Index] = (Header->Entry[Index].Lba + Header->Entry[Index].NumSectors);

        PartitionStart[Index] *= Volume->BytesPerSector;
        PartitionEnd[Index] *= Volume->BytesPerSector;

      }

    } else if (Header->Entry[Index].ChsStart.Sectors != 0) {

      // (Calculate CHS values)

      // Here, we're assuming 63 sectors per head and 255 heads per
      // cylinder, as well as a sector size of 512 bytes.

      if (Header->Entry[Index].ChsEnd.Sectors != 0) {

        PartitionStart[Index] = ((uint64)Header->Entry[Index].ChsStart.Sectors - 1 +
                                ((uint64)Header->Entry[Index].ChsStart.Heads * 63) +
                                ((uint64)Header->Entry[Index].ChsStart.Cylinders * 255 * 63));

        PartitionEnd[Index] = ((uint64)Header->Entry[Index].ChsEnd.Sectors - 1 +
                              ((uint64)Header->Entry[Index].ChsEnd.Heads * 63) +
                              ((uint64)Header->Entry[Index].ChsEnd.Cylinders * 255 * 63));

        PartitionStart[Index] *= 512;
        PartitionEnd[Index] *= 512;

      }

    }

  }

  // (2) Check the calculated limits, and make sure they don't overlap
  // with each other, or exceed the volume's sector limit.

  auto NumEmptyEntries = 0;
  const uint64 SectorLimit = (Volume->BytesPerSector * Volume->NumSectors);

  for (auto Index = 0; Index < 4; Index++) {

    // (If this partition entry's type is none (or 00h), increment
    // `NumEmptyEntries` and move onto the next entry)

    if (Header->Entry[Index].Type == MbrEntryType_None) {

      NumEmptyEntries++;
      continue;

    }

    // (If the start of the partition contains the first sector, we
    // can probably assume we don't have a valid MBR)

    if (PartitionStart[Index] == 0) {
      return false;
    }

    // (If the end of the partition exceeds the volume's sector limit,
    // and it isn't a GPT Protective (or EEh) entry, return false)

    if (PartitionEnd[Index] > SectorLimit) {

      if (Header->Entry[Index].Type != MbrEntryType_Gpt) {
        return false;
      }

    }

    // (Finally, if the start of the partition doesn't come before it
    // supposedly ends, we can assume we likely don't have a valid
    // partition table either)

    if (PartitionStart[Index] >= PartitionEnd[Index]) {
      return false;
    }

  }

  // (3) Make sure that the MBR's partition table contains *at least*
  // a single, non-empty entry

  if (NumEmptyEntries == 4) {
    return false;
  }

  // [DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG]

  Message(Kernel, "Volume %d is partitioned.", VolumeNum);

  for (auto i = 0; i < 4; i++) {

    if (Header->Entry[i].Type == 0) continue;

    Message(Ok, "[%d].Attributes = %xh; [%d].Type = %xh", i,
                Header->Entry[i].Attributes, i, Header->Entry[i].Type);

    Message(Ok, "[%d].ChsStart @ (Cylinders = %xh, Heads = %xh, Sectors = %xh)", i,
                (uint64)Header->Entry[i].ChsStart.Cylinders,
                (uint64)Header->Entry[i].ChsStart.Heads,
                (uint64)Header->Entry[i].ChsStart.Sectors);

    Message(Ok, "[%d].ChsEnd @ (Cylinders = %xh, Heads = %xh, Sectors = %xh)", i,
                (uint64)Header->Entry[i].ChsEnd.Cylinders,
                (uint64)Header->Entry[i].ChsEnd.Heads,
                (uint64)Header->Entry[i].ChsEnd.Sectors);

    Message(Ok, "[%d].Lba = %xh, [%d].NumSectors = %xh", i,
                Header->Entry[i].Lba, i, Header->Entry[i].NumSectors);

  }

  // If we've managed to pass all of these checks, that means we're
  // likely dealing with a valid MBR, so let's return `true`.

  return true;

}



// (TODO - Include a function to validate an *individual* GPT header)

[[nodiscard]] static bool ValidateGptHeader(gptHeader* Header, uint16 VolumeNum) {

  // Before we do anything else, we need to check if the signature
  // is valid - it should match `gptHeaderSignature`.

  if (Header->Signature != gptHeaderSignature) {
    return false;
  }

  // Additionally, we want to check if the revision is valid; in that
  // case, it should be at least `gptHeaderRevision`.

  if (Header->Revision < gptHeaderRevision) {
    return false;
  }

  // (TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO - CRC32)
  // (TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO - CRC32)
  // (TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO - CRC32)

  // We also want to check whether the size of the header is valid;
  // it should be at least `sizeof(gptHeader)`, but it shouldn't
  // exceed the size of one sector/LBA either.

  if (Header->Size < sizeof(gptHeader)) {
    return false;
  } else if (Header->Size > VolumeList[VolumeNum].BytesPerSector) {
    return false;
  }

  // Now that we know the header size is likely valid, we can move onto
  // the next step: calculating the checksum of the header.

  // In order to do this, we need to clear out `Header->Crc32` before
  // doing the calculation with CalculateCrc32() - then, we just
  // have to compare the return value.

  const uint32 Checksum = Header->Crc32;
  Header->Crc32 = 0;

  const uint32 CalculatedChecksum = CalculateCrc32(Header, Header->Size);
  Header->Crc32 = Checksum;

  if (Checksum != CalculatedChecksum) {
    return false;
  }

  // Next, we want to check whether the primary and backup header LBA
  // values are accurate - they should match 1 and `BackupLba`
  // respectively.

  auto BackupLba = (VolumeList[VolumeNum].NumSectors - 1);

  if (Header->HeaderLba != 1) {
    return false;
  } else if (Header->BackupLba != BackupLba) {
    return false;
  }

  // In theory, that should be everything that's *officially* needed
  // to verify a GPT header.. but just in case:

  // (1) Check that `PartitionLba` comes after Header->HeaderLba:

  if (Header->PartitionLba <= Header->HeaderLba) {
    return false;
  }

  // (2) Check that the number of partition entries isn't zero:

  if (Header->NumPartitions == 0) {
    return false;
  }

  // (3) Check that the partition entry size is a multiple of 8:

  if ((Header->PartitionEntrySize % 8) != 0) {
    return false;
  }

  // (4) Make sure that everything after the table is zeroed out:
  // (This only scans within the current block)

  auto SectorSize = VolumeList[VolumeNum].BytesPerSector;
  uint8* RawHeader = (uint8*)Header;

  for (auto Index = sizeof(gptHeader); Index < SectorSize; Index++) {

    if (RawHeader[Index] != 0) {
      return false;
    }

  }

  // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

  Message(Kernel, "Volume %d has a valid GPT header.", VolumeNum);

  Message(Ok, "Signature = %xh | Revision = %xh | Size = %xh | Crc32 = %xh",
    Header->Signature, Header->Revision, Header->Size, Header->Crc32);

  Message(Ok, "[Header,Backup]Lba = [%xh,%xh] | [First,Last]UsableLba = [%xh,%xh]",
    Header->HeaderLba, Header->BackupLba, Header->FirstUsableLba, Header->LastUsableLba);

  Message(Ok, "DiskUuid = {%x, {%x,%x}, {%x,%x,%x,%x,%x,%x,%x,%x}}",
    Header->DiskUuid.Uuid_A, Header->DiskUuid.Uuid_B[0], Header->DiskUuid.Uuid_B[1],
    Header->DiskUuid.Uuid_C[0], Header->DiskUuid.Uuid_C[1], Header->DiskUuid.Uuid_C[2],
    Header->DiskUuid.Uuid_C[3], Header->DiskUuid.Uuid_C[4], Header->DiskUuid.Uuid_C[5],
    Header->DiskUuid.Uuid_C[6], Header->DiskUuid.Uuid_C[7]);

  Message(Ok, "PartitionLba = %xh | NumPartitions = %d",
    Header->PartitionLba, Header->NumPartitions);

  Message(Ok, "PartitionEntrySize = %xh | PartitionCrc32 = %xh",
    Header->PartitionEntrySize, Header->PartitionCrc32);

  // Now that we've passed all of these checks, we can return `true`
  // to indicate that the given GPT header is likely valid.

  return true;

}



// (TODO - Include a function to process a partitioned volume, and
// identify (as well as add) each partition it contains)

// Dynamically updates `NumVolumes`, *returning the old value*, and adds
// partitions (obviously) - in a for loop, you'd do something like:
// `for (uint16 Index = ReturnVal; Index < NumVolumes; Index++)`

static uint16 DetectPartitionMap(mbrHeader* Mbr, uint16 VolumeNum) {

  // Before we do anything else, let's see if the disk is MBR- or
  // GPT-formatted, by checking for the existence of a GPT
  // Protective partition (type = EEh).

  // (Even GPT-formatted devices still contain a protective MBR for
  // compatibility reasons, so it's safe to assume we have one)

  bool GptPartitionMap = false;

  for (auto EntryNum = 0; EntryNum < 4; EntryNum++) {

    if (Mbr->Entry[EntryNum].Type == MbrEntryType_Gpt) {

      GptPartitionMap = true;
      break;

    }

  }

  // (Save the current number of volumes on the disk)

  const uint16 SaveNumVolumes = NumVolumes;

  // If we *are* dealing with a GPT-formatted disk, then we'll need to
  // read from more than just the bootsector; more specifically, we'll
  // need to read the GPT headers from the second and last LBAs.

  // (Unlike MBR, GPT requires a backup of the header and partition
  // tables, which is always located at the end of the disk)

  bool UseBackupGptHeader = false;
  volumeInfo* Volume = &VolumeList[VolumeNum];
  const uintptr SectorSize = Volume->BytesPerSector;

  [[maybe_unused]] gptHeader* PrimaryGptHeader = NULL;
  [[maybe_unused]] gptHeader* BackupGptHeader = NULL;

  if (GptPartitionMap == true) {

    // (Allocate space for `PrimaryGptHeader` and `BackupGptHeader`)

    PrimaryGptHeader = (gptHeader*)(Allocate(&SectorSize));
    BackupGptHeader = (gptHeader*)(Allocate(&SectorSize));

    if ((PrimaryGptHeader == NULL) || (BackupGptHeader == NULL)) {
      goto Cleanup;
    }

    // (Read the second (LBA 1) and last (LBA -1) sectors from the disk
    // into their respective headers)

    bool ReadPrimaryGptHeader = ReadDisk((void*)PrimaryGptHeader,
                                         (1 * SectorSize),
                                         SectorSize, VolumeNum);

    bool ReadBackupGptHeader = ReadDisk((void*)BackupGptHeader,
                                        ((Volume->NumSectors - 1) * SectorSize),
                                        SectorSize, VolumeNum);

    if ((ReadPrimaryGptHeader == false) || (ReadBackupGptHeader == false)) {
      goto Cleanup;
    }

    // Finally, now that we've successfully read those headers, we can
    // pass them onto ValidateGptHeader() to make sure they're valid.

    // (We first check the primary GPT header, and if that's corrupt,
    // we *then* check the backup GPT header as well)

    if (ValidateGptHeader(PrimaryGptHeader, VolumeNum) == false) {

      if (ValidateGptHeader(BackupGptHeader, VolumeNum) == false) {
        goto Cleanup;
      } else {
        UseBackupGptHeader = true;
      }

    }

  }

  // Finally, now that we have the necessary data, let's obtain each
  // of the volume's partitions - we'll need to create a separate
  // volume for each one, like this:

  if (GptPartitionMap == true) {

    // (TODO - Handle GPT partitions; limit is 32)

    // I might need something to actually check if the partition tables
    // are valid or corrupt - the headers aren't, but I can't say
    // the same about the actual partition tables :(

  } else {

    // (TODO - Handle MBR partitions; limit is 4)

  }



  // (No matter what, we *have* to free what we allocated)

  Cleanup:

  if (PrimaryGptHeader != NULL) {
    [[maybe_unused]] bool Result = Free((void*)PrimaryGptHeader, &SectorSize);
  }

  if (BackupGptHeader != NULL) {
    [[maybe_unused]] bool Result = Free((void*)BackupGptHeader, &SectorSize);
  }

  return SaveNumVolumes;

}



// (TODO - Include a function to read through a volume, and identify +
// automatically add any relevant partitions.)

[[nodiscard]] bool InitializeFsSubsystem(void) {

  // Before we do anything else, we need to check whether the disk
  // subsystem has been initialized, and whether there are any
  // volumes in the system.

  if (DiskInfo.IsEnabled == false) {
    return false;
  } else if (NumVolumes == 0) {
    return false;
  }

  // Next, we need to iterate through each applicable volume on the
  // system, and attempt to identify its type.

  auto NumUsableVolumes = 0;
  const uint16 VolumeLimit = NumVolumes;

  for (uint16 Index = 0; Index < VolumeLimit; Index++) {

    // (Read the first 512 bytes of the current volume; this corresponds
    // to the bootsector of the volume, and helps us identify a few
    // things)

    uint8 Bootsector[512];

    if (ReadDisk((void*)Bootsector, 0, 512, Index) == true) {
      NumUsableVolumes++;
    } else {
      continue;
    }

    // (Try to identify the volume type: it can either be *partitioned*,
    // in which case it should contain an MBR, or *unpartitioned*, in
    // which case it should contain a filesystem.)

    // Even a GPT-partitioned disk must still have a protective MBR at
    // the beginning, so it should be safe to assume one exists.

    mbrHeader* Mbr = (mbrHeader*)Bootsector;
    bool IsPartitioned = ValidateMbrHeader(Mbr, Index);

    // Now that we know for sure whether the volume is partitioned
    // or not, we can move onto processing it.

    // (If the volume is partitioned, add each partition to the
    // volume list)

    if (IsPartitioned == true) {
      DetectPartitionMap(Mbr, Index);
    }

    // TODO - Add a function to do that

    // (If the volume isn't partitioned *or* we just finished
    // adding each partition, process the partition itself)
    // TODO - Add a function to do that

  }

  // If we didn't find any usable volumes, then we should return
  // false.

  if (NumUsableVolumes == 0) {
    return false;
  } else {
    return true;
  }

}



// (TODO - Include a function to initialize the filesystem subsystem..
// or, better, this is the disk subsystem, to just do the things above
// and try to automatically identify partitions and all)
