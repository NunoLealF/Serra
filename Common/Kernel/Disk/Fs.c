// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Memory/Memory.h"
#include "Disk.h"

// (TODO - Include MBR and GPT-related functions (and typedefs..?))



// (TODO - Include a function to process an unpartitioned volume / an
// individual filesystem)



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

    bool IsPartitioned = false;
    mbrHeader* Mbr = (mbrHeader*)((uintptr)Bootsector + 446);

    // (Check if the volume is partitioned - we skip any volumes that
    // don't have a valid signature, or that are already indicated
    // as representing a partition and not an entire device)

    #include "../Libraries/Stdio.h" // (DEBUG)

    if ((Mbr->Signature == 0xAA55) && (VolumeList[Index].IsPartition == false)) {

      // Before we do anything else, let's go through each partition
      // entry, and see if they make sense.

      // (Calculate the start and limit of each partition, in bytes; we
      // check both CHS (assuming 512-byte sectors) and LBA)

      uint64 PartitionStart[4] = {0};
      uint64 PartitionLimit[4] = {0};

      for (auto EntryNum = 0; EntryNum < 4; EntryNum++) {

        // (If this entry doesn't represent any partition type, skip it)

        if (Mbr->Entry[EntryNum].Type == MbrEntryType_None) {
          continue;
        }

        // (Calculate the CHS (Cylinder/Head/Sector) start+limit, in bytes)

        // WARNING - This assumes 63 sectors per head, and 16 heads per
        // cylinder, which may not be correct; because of that, these
        // limits are only used when LBA ones aren't available.

        uint64 ChsStart = 0;
        uint64 ChsLimit = 0;

        if (Mbr->Entry[EntryNum].ChsStart.Sectors != 0) {

          if (Mbr->Entry[EntryNum].ChsEnd.Sectors != 0) {

            ChsStart = ((uint64)Mbr->Entry[EntryNum].ChsStart.Sectors - 1 +
                       ((uint64)Mbr->Entry[EntryNum].ChsStart.Heads * 63) +
                       ((uint64)Mbr->Entry[EntryNum].ChsStart.Cylinders * 16 * 63));

            ChsLimit = ((uint64)Mbr->Entry[EntryNum].ChsEnd.Sectors - 1 +
                       ((uint64)Mbr->Entry[EntryNum].ChsEnd.Heads * 63) +
                       ((uint64)Mbr->Entry[EntryNum].ChsEnd.Cylinders * 16 * 63));

            ChsStart *= 512;
            ChsLimit *= 512;

          }

        }

        // (Calculate the LBA (Logical Block Addressing) start+limit, in bytes)

        uint64 LbaStart = 0;
        uint64 LbaLimit = 0;

        if (Mbr->Entry[EntryNum].Lba != 0) {

          if (Mbr->Entry[EntryNum].NumSectors != 0) {

            LbaStart = ((uint64)Mbr->Entry[EntryNum].Lba);

            LbaLimit = ((uint64)Mbr->Entry[EntryNum].Lba +
                        (uint64)Mbr->Entry[EntryNum].NumSectors);

            LbaStart *= VolumeList[Index].BytesPerSector;
            LbaLimit *= VolumeList[Index].BytesPerSector;

          }

        }

        // (Depending on whether `LbaLimit` is zero, either use the
        // LBA or the CHS values)

        if (LbaLimit != 0) {

          PartitionStart[EntryNum] = LbaStart;
          PartitionLimit[EntryNum] = LbaLimit;

        } else {

          PartitionStart[EntryNum] = ChsStart;
          PartitionLimit[EntryNum] = ChsLimit;

        }

      }

      // (Check the calculated limits, and make sure they don't overlap
      // with each other, or exceed the maximum number of sectors)

      IsPartitioned = true;

      const uint64 Limit = (VolumeList[Index].BytesPerSector *
                            VolumeList[Index].NumSectors);

      auto NumEmptyEntries = 0;

      for (auto EntryNum = 0; EntryNum < 4; EntryNum++) {

        // (If the partition entry's type isn't 'none', and the start
        // is zero, we can probably assume we don't have a valid
        // MBR on our hands)

        if (Mbr->Entry[EntryNum].Type != MbrEntryType_None) {

          if (PartitionStart[EntryNum] == 0) {

            IsPartitioned = false;
            break;

          }

        } else {

          NumEmptyEntries++;

        }

        // (If the partition limit exceeds the drive's actual limit, and
        // it isn't a GPT Protective partition, we can assume we don't
        // have a valid MBR)

        if (PartitionLimit[EntryNum] > Limit) {

          if (Mbr->Entry[EntryNum].Type != MbrEntryType_Gpt) {

            IsPartitioned = false;
            break;

          }

        }

        // (Finally, if the start ever exceeds the limit, we can assume
        // that we don't have a valid MBR either)

        if (PartitionStart[EntryNum] > PartitionLimit[EntryNum]) {

          IsPartitioned = false;
          break;

        }

      }

      // (Lastly, check if all entries are empty, since that indicates
      // the lack of an MBR at all)

      if (NumEmptyEntries == 4) {
        IsPartitioned = false;
      }

      // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

      Message(Kernel, "Volume %d - IsPartitioned = `%s`",
                       Index, (IsPartitioned == true) ? "true" : "false");

      if (IsPartitioned == true) {

        Message(Ok, "Start[0..3] = %xh, %xh, %xh, %xh", PartitionStart[0],
                    PartitionStart[1], PartitionStart[2], PartitionStart[3]);
        Message(Ok, "Limit[0..3] = %xh, %xh, %xh, %xh", PartitionLimit[0],
                    PartitionLimit[1], PartitionLimit[2], PartitionLimit[3]);
        Message(Ok, "Type[0..3] = %xh, %xh, %xh, %xh", Mbr->Entry[0].Type,
                    Mbr->Entry[1].Type, Mbr->Entry[2].Type, Mbr->Entry[3].Type);

      } else {

        Message(Warning, "Not partitioned (couldn't find MBR).");

      }

    } else {

      Message(Kernel, "Volume %d - IsPartitioned = `false`", Index);
      Message(Warning, "Not partitioned (not bootable, or already represents a partition).");

    }

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
