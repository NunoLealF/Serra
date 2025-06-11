// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Memory/Memory.h"
#include "Disk.h"

// (TODO - Include a function to process an unpartitioned volume / an
// individual filesystem)



// (TODO - Include a function to validate an MBR header)

[[nodiscard]] static bool ValidateMbrHeader(mbrHeader* Header, uint16 VolumeNum) {

  // (Check that `Bootsector` is a valid pointer, and that `VolumeNum`
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

  #include "../Libraries/Stdio.h"

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



// (TODO - Include a function to validate a GPT header)

[[nodiscard]] static bool ValidateGptHeader(gptHeader* PrimaryHeader, gptHeader* BackupHeader, uint16 VolumeNum) {
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

  // If we *are* dealing with a GPT-formatted disk, then we'll need
  // to read from more than just the 512-byte bootsector (`Mbr`);
  // more specifically, we'll need to read LBAs 1 through 33.

  // (If `GptPartitionMap` is true, allocate a buffer wide enough
  // to store 33 sectors in)

  // TODO - The documentation doesn't actually say that - you
  // need to read the second LBA (LBA 1), which is the GPT header,
  // which then has the correct LBA amounts and such

  // TODO TODO TODO TODO (Fix the thing above) TODO TODO TODO TODO

  const uint16 SaveNumVolumes = NumVolumes;
  const uintptr GptSize = (VolumeList[VolumeNum].BytesPerSector * 33);
  void* Gpt = NULL;

  if (GptPartitionMap == true) {

    Gpt = Allocate(&GptSize);

    if (Gpt == NULL) {
      return SaveNumVolumes;
    }

  }

  // (Read the data into the area we just allocated)

  if (GptPartitionMap == true) {

    auto Offset = VolumeList[VolumeNum].BytesPerSector;
    auto Size = (Offset * 33);

    if (ReadDisk(Gpt, Offset, Size, VolumeNum) == false) {
      goto Cleanup;
    }

  }

  // Finally, now that we have the necessary data, let's obtain each
  // of the volume's partitions - we'll need to create a separate
  // volume for each one, like this:

  if (GptPartitionMap == true) {

    // (TODO - Handle GPT partitions; limit is 32)

  } else {

    // (TODO - Handle MBR partitions; limit is 4)

  }


  // (If applicable, free what we allocated, and return the value
  // we saved earlier on)

  Cleanup:

  if (Gpt != NULL) {
    [[maybe_unused]] bool Result = Free(Gpt, &GptSize);
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

    mbrHeader* Mbr = (mbrHeader*)((uintptr)Bootsector + 446);
    bool IsPartitioned = ValidateMbrHeader(Mbr, Index);

    // Now that we know for sure whether the volume is partitioned
    // or not, we can move onto processing it.

    // (If the volume is partitioned, add each partition to the
    // volume list)
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
