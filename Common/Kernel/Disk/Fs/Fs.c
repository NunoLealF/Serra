// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Memory/Memory.h"
#include "../Disk.h"
#include "Fs.h"

// (NOTE - This is for debug/information messages only)

#include "../../Libraries/Stdio.h"



// (TODO - Include a function to process an unpartitioned volume / an
// individual filesystem)



// (TODO - Include a function to convert a CHS address into an LBA, using
// predetermined `SectorsPerHead` and `HeadsPerCylinder` values)

// (WARNING - This uses a predetermined number of sectors per head, and
// heads per cylinder; that's not out of laziness, it's just that
// there's no standard way to figure that out)

// The *vast* majority of MBRs out there support LBA values, which
// are much more reliable - this is just a fallback.

static uint64 ConvertChsToLba(chsAddress Chs) [[reproducible]] {

  // (Keep in mind that the number of sectors in the CHS format
  // always needs to be decremented by one)

  #define SectorsPerHead 63
  #define HeadsPerCylinder 255

  return ((Chs.Sectors - 1) + (Chs.Heads * SectorsPerHead) +
          (Chs.Cylinders * SectorsPerHead * HeadsPerCylinder));

}



// (TODO - Include a function to get the sector/LBA offset of an MBR
// partition entry (this is just in case LBA isn't available))

static uint64 CalculateMbrOffset(mbrHeader* Header, uint16 Partition) {

  // (If LBA values are available, use those - otherwise, try to
  // use the CHS values (using ConvertChsToLba()))

  if (Header->Entry[Partition].Lba != 0) {
    return Header->Entry[Partition].Lba;
  } else {
    return ConvertChsToLba(Header->Entry[Partition].ChsStart);
  }

}



// (TODO - Same as above, but for the size)

static uint64 CalculateMbrSize(mbrHeader* Header, uint16 Partition) {

  // (Depending on whether CHS or LBA is available, calculate
  // the necessary size)

  if (Header->Entry[Partition].NumSectors != 0) {

    // (If LBA values are available, so is the `NumSectors` field, so
    // we just need to return that)

    return Header->Entry[Partition].NumSectors;

  } else {

    // (Otherwise, we need to convert the start and end CHS values to
    // LBA, and return the difference between them)

    uint64 Start = ConvertChsToLba(Header->Entry[Partition].ChsStart);
    uint64 End = ConvertChsToLba(Header->Entry[Partition].ChsEnd);

    return (End + Start - 1);

  }

}



// (TODO - Include a function to convert an MBR partition type into one
// understood by volumeInfo.Type)

static uint16 ConvertMbrPartitionType(const uint8 PartitionType) [[reproducible]] {

  // (Compare `PartitionType` with our supported MBR partition types)

  switch (PartitionType) {

    // (Completely ignore empty and GPT protective partitions)

    case MbrPartitionType_None:
      [[fallthrough]];

    case MbrPartitionType_Gpt:
      return VolumeType_Unknown;
      break;

    // (FAT-12, FAT-16, FAT-32 and the EFI System Partition are
    // all equivalent to `VolumeType_Partition_Fat`)

    case MbrPartitionType_Fat12:
      [[fallthrough]];

    case MbrPartitionType_Fat16_A:
      [[fallthrough]];

    case MbrPartitionType_Fat16_B:
      [[fallthrough]];

    case MbrPartitionType_Fat16_C:
      [[fallthrough]];

    case MbrPartitionType_Fat32_A:
      [[fallthrough]];

    case MbrPartitionType_Fat32_B:
      [[fallthrough]];

    case MbrPartitionType_Esp:
      return VolumeType_Partition_Fat;
      break;

  }

  // (Otherwise, return `VolumeType_Partition_Unknown`, in order to
  // indicate that we don't support the given partition type)

  return VolumeType_Partition_Unknown;

}



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

    if (Header->Entry[Index].Type == MbrPartitionType_None) {
      continue;
    }

    // (Calculate the value of PartitionStart[] and PartitionEnd[]
    // using CalculateMbrOffset() and CalculateMbrSize())

    PartitionStart[Index] = CalculateMbrOffset(Header, Index);

    PartitionEnd[Index] = (CalculateMbrOffset(Header, Index) +
                           CalculateMbrSize(Header, Index));

  }

  // (2) Check the calculated limits, and make sure they don't overlap
  // with each other, or exceed the volume's sector limit.

  auto NumEmptyEntries = 0;
  const uint64 SectorLimit = (Volume->BytesPerSector * Volume->NumSectors);

  for (auto Index = 0; Index < 4; Index++) {

    // (If this partition entry's type is none (or 00h), increment
    // `NumEmptyEntries` and move onto the next entry)

    if (Header->Entry[Index].Type == MbrPartitionType_None) {

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

      if (Header->Entry[Index].Type != MbrPartitionType_Gpt) {
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

  // If we've managed to pass all of these checks, that means we're
  // likely dealing with a valid MBR, so let's return `true`.

  return true;

}



// (TODO - Include a function to convert an MBR partition type into one
// understood by volumeInfo.Type)

static uint16 ConvertGptPartitionType(const genericUuid PartitionType) [[reproducible]] {

  // (Define a macro that can be used to compare `PartitionType` with
  // another partition type (defined as `genericUuid`))

  // C doesn't allow you to directly compare structures, and since UUIDs
  // are represented as such, we need to use Memcmp(), like this:

  #define MatchesPartitionType(Type) ((Memcmp(&Type, &PartitionType, sizeof(genericUuid)) == 0) ? true : false)

  // (EFI System Partitions are always `VolumeType_Partition_Fat`)

  if (MatchesPartitionType(GptPartitionType_Esp)) {
    return VolumeType_Partition_Fat;
  }

  // (Microsoft and Linux basic data partitions can be FAT, but aren't
  // guaranteed to, so, use `VolumeType_Partition_BasicData`)

  if (MatchesPartitionType(GptPartitionType_BasicData)) {
    return VolumeType_Partition_BasicData;
  } else if (MatchesPartitionType(GptPartitionType_LinuxData)) {
    return VolumeType_Partition_BasicData;
  }

  // (If we've gotten this far, then it doesn't match any of the supported
  // types, so we can just return `VolumeType_Partition_Unknown`)

  return VolumeType_Partition_Unknown;

}



// (TODO - Include a function to validate an *individual* GPT header)

[[nodiscard]] static bool ValidateGptHeader(gptHeader* Header, uint16 VolumeNum) {

  // Before we do anything else, we need to check if the signature
  // is valid - it should match `gptHeaderSignature`.

  if (Header->Signature != gptHeaderSignature) {
    return false;
  }

  // Additionally, we want to check if the revision is valid; in our
  // case, it should be at least `gptHeaderRevision`.

  if (Header->Revision < gptHeaderRevision) {
    return false;
  }

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
  // have to compare the return value:

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

  // (1) Check that `PartitionLba` comes after Header->HeaderLba,
  // and before Header->BackupLba:

  if (Header->PartitionLba <= Header->HeaderLba) {
    return false;
  } else if (Header->PartitionLba >= Header->BackupLba) {
    return false;
  }

  // (2) Check that the number of partition entries isn't zero:

  if (Header->NumPartitions == 0) {
    return false;
  }

  // (3) Check that the partition entry size is at least 128, as
  // well as a multiple of 8:

  if (Header->PartitionEntrySize < 128) {
    return false;
  } else if ((Header->PartitionEntrySize % 8) != 0) {
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

    if (Mbr->Entry[EntryNum].Type == MbrPartitionType_Gpt) {

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
        Message(Warning, "Using backup GPT header.");
        UseBackupGptHeader = true;
      }

    }

  }

  // Finally, now that we have the necessary data, let's obtain each
  // of the volume's partitions - we'll need to create a separate
  // volume for each one, like this:

  uintptr GptPartitionArraySize = 0;
  [[maybe_unused]] gptPartition* GptPartitionArray = NULL;

  if (GptPartitionMap == true) {

    // (Depending on the value of `UseBackupGptHeader`, pick the
    // right GPT header)

    gptHeader* Header = PrimaryGptHeader;

    if (UseBackupGptHeader == true) {
      Header = BackupGptHeader;
    }

    // Before we do anything else, we need to load the partition entry
    // array from the disk.

    // (Calculate the necessary size to hold each partition entry, and
    // allocate a buffer of that size)

    GptPartitionArraySize = (Header->NumPartitions * Header->PartitionEntrySize);
    GptPartitionArray = (gptPartition*)(Allocate(&GptPartitionArraySize));

    // (Check if the allocation was successful)

    if (GptPartitionArray == NULL) {
      goto Cleanup;
    }

    // (Load in the partition entry array)

    bool ReadGptPartitionArray = ReadDisk((void*)GptPartitionArray,
                                          (Header->PartitionLba * SectorSize),
                                          GptPartitionArraySize, VolumeNum);

    if (ReadGptPartitionArray == false) {
      goto CleanupPartition;
    }

    // (Calculate the CRC-32 checksum of the partition entry array in
    // order to make sure that it's actually valid)

    auto Checksum = CalculateCrc32(GptPartitionArray, GptPartitionArraySize);

    if (Checksum != Header->PartitionCrc32) {
      goto CleanupPartition;
    }

    // Now that we *know* we have a valid partition table array, we can
    // move onto the next part - actually interpreting each partition.

    for (uint32 Index = 0; Index < Header->NumPartitions; Index++) {

      // (Declare initial variables)

      const gptPartition* Partition = &GptPartitionArray[Index];
      const genericUuid Type = Partition->Type;

      // If this partition entry is empty (`GptPartitionType_None`),
      // then move onto the next entry.

      if (Memcmp(&Type, &GptPartitionType_None, sizeof(genericUuid)) == 0) {
        continue;
      }

      // Otherwise, create a volume for the current partition entry,
      // updating `NumVolumes` in the process:

      auto VolumeLimit = (sizeof(VolumeList) / sizeof(volumeInfo));

      if (NumVolumes < VolumeLimit) {

        // (Add a new volume to the list, and increment `NumPointers`)

        volumeInfo* PartitionVolume = &VolumeList[NumVolumes];
        NumVolumes++;

        // (Fill out drive-specific values, copying them from `Volume`)

        PartitionVolume->Method = Volume->Method;
        PartitionVolume->Drive = Volume->Drive;

        PartitionVolume->Alignment = Volume->Alignment;
        PartitionVolume->BytesPerSector = Volume->BytesPerSector;
        PartitionVolume->MediaId = Volume->MediaId;

        // (Fill out partition-specific values from our GPT partition entry)

        PartitionVolume->IsPartition = true;
        PartitionVolume->Partition = (uint16)Index;

        PartitionVolume->NumSectors = (1 + Partition->EndingLba - Partition->StartingLba);
        PartitionVolume->PartitionOffset = Partition->StartingLba;

        PartitionVolume->Type = ConvertGptPartitionType(Partition->Type);

      } else {

        // (If we've exceeded the volume limit, stop)

        break;

      }

    }

    // (Free the buffer we allocated for the partition array, if applicable)

    CleanupPartition:

    if (GptPartitionArray != NULL) {
      [[maybe_unused]] bool Result = Free((void*)GptPartitionArray, &GptPartitionArraySize);
    }

    goto Cleanup;

  } else {

    // If we're dealing with an MBR-partitioned disk, then we've already
    // gone through the validation process, so we can skip right
    // onto adding each partition to the volume list:

    for (auto Index = 0; Index < 4; Index++) {

      // If the current partition entry is empty or GPT protective, move
      // onto the next entry in the list.

      if (Mbr->Entry[Index].Type == MbrPartitionType_None) {
        continue;
      } else if (Mbr->Entry[Index].Type == MbrPartitionType_Gpt) {
        continue;
      }

      // Otherwise, create an entry in the volume list (as long as
      // there's enough space, obviously):

      auto VolumeLimit = (sizeof(VolumeList) / sizeof(volumeInfo));

      if (NumVolumes < VolumeLimit) {

        // (Add a new volume to the list, and increment `NumPointers`)

        volumeInfo* PartitionVolume = &VolumeList[NumVolumes];
        NumVolumes++;

        // (Fill out drive-specific values, copying them from `Volume`)

        PartitionVolume->Method = Volume->Method;
        PartitionVolume->Drive = Volume->Drive;

        PartitionVolume->Alignment = Volume->Alignment;
        PartitionVolume->BytesPerSector = Volume->BytesPerSector;
        PartitionVolume->MediaId = Volume->MediaId;

        // (Fill out partition-specific values from our MBR partition entry)

        PartitionVolume->IsPartition = true;
        PartitionVolume->Partition = Index;

        PartitionVolume->NumSectors = CalculateMbrSize(Mbr, Index);
        PartitionVolume->PartitionOffset = CalculateMbrOffset(Mbr, Index);

        PartitionVolume->Type = ConvertMbrPartitionType(Mbr->Entry[Index].Type);

      } else {

        // (If we've exceeded the volume limit, stop)

        break;

      }

    }

  }

  // Finally, now that we're done adding partition volumes, we also want
  // to update *our own* volume entry with the correct type:

  if (GptPartitionMap == true) {
    Volume->Type = VolumeType_Gpt;
  } else {
    Volume->Type = VolumeType_Mbr;
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

    // Now that we know for sure whether the volume is partitioned or
    // not, we can move onto the next step - processing it.

    if (IsPartitioned == true) {

      // (If the volume is *partitioned*, try to interpret the partition
      // map, and add each entry to the volume list)

      Message(Kernel, "Preparing to process volume (%d).", (uint64)Index);

      uint16 OldNumVolumes = DetectPartitionMap(Mbr, Index);

      Message(Info, "Volume (%d) uses %s partition map.", (uint64)Index,
                    ((VolumeList[Index].Type == VolumeType_Mbr) ? "an MBR" : "a GPT"));

      for (auto VolumeIndex = OldNumVolumes; VolumeIndex < NumVolumes; VolumeIndex++) {

        Message(Ok, "Successfully added a volume entry (%d) for method(%d)drive(%d)partition(%d)",
                    (uint64)OldNumVolumes, (uint64)VolumeList[VolumeIndex].Method,
                    (uint64)VolumeList[VolumeIndex].Drive,
                    (uint64)VolumeList[VolumeIndex].Partition);

        Message(Info, "Volume entry (%d) is a partition of type %xh (see volumeInfo{}.Type)",
                      (uint64)Index, (uint64)VolumeList[VolumeIndex].Type);

        Message(Info, "Volume entry (%d) has a sector size of %d bytes, and %d sectors",
                      (uint64)Index, (uint64)VolumeList[VolumeIndex].BytesPerSector,
                      (uint64)VolumeList[VolumeIndex].NumSectors);

        uint64 Size = (VolumeList[VolumeIndex].BytesPerSector *
                       VolumeList[VolumeIndex].NumSectors);

        Message(Info, "(Which corresponds to a total size of %d MiB, or %d GiB)",
                      (Size / (1024 * 1024)), (Size / (1024 * 1024 * 1024)));

      }

      Print("\n\r", false, 0x07);

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
