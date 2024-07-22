// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Disk.h"

// Global variables

uint8 DriveNumber;

uint16 LogicalSectorSize;
uint16 PhysicalSectorSize;

/* realModeTable* ReadSector())

   Inputs: uint16 NumBlocks - The number of sectors (blocks) that we want to load.
           uint32 Address - The 48-bit memory address we want to load *to*.
           uint64 Offset - The LBA (sector number) we want to start loading *from*.

   Outputs: realModeTable* - This function uses a real mode BIOS interrupt (int 13h, ah 42h) in
   order to actually load anything, so this is just the output of that; make sure to check it
   for the carry flag (and for error codes in the ah register)

   This function loads data from the disk, using the BIOS function (int 13h, ah 42h);
   specifically, it loads [NumBlocks] sectors starting from the LBA [Offset] (on the disk
   indicated by [DriveNumber]) to the memory address at [Address].

   Since it isn't really necessary, this function doesn't support 64-bit memory addresses, but
   the disk address packet format used by this BIOS function does support it (although it's
   unclear whether many BIOSes support it at all).

*/

realModeTable* ReadSector(uint16 NumBlocks, uint32 Address, uint64 Offset) {

  // Create a eddDiskAddressPacket structure with the data we got.

  eddDiskAddressPacket DiskAddressPacket;

  DiskAddressPacket.Size = 0x10;
  DiskAddressPacket.Reserved = 0x00;

  DiskAddressPacket.Address_Segment = (Address >> 4);
  DiskAddressPacket.Address_Offset = (Address & 0x0F);

  DiskAddressPacket.NumBlocks = NumBlocks;
  DiskAddressPacket.Lba_Low = (Offset & 0xFFFFFFFF);
  DiskAddressPacket.Lba_High = (Offset >> 32);

  // Now, we want to prepare to actually read from disk - specifically, we'll be using the
  // BIOS function (int 13h, ah 42h).

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = (0x42 << 8);
  Table->Edx = DriveNumber;

  Table->Ds = (uint16)((int)(&DiskAddressPacket) >> 4);
  Table->Si = (uint16)((int)(&DiskAddressPacket) & 0x0F);

  Table->Int = 0x13;
  RealMode();

  // Finally, we want to return our real mode table (which should be able to indicate a variety
  // of things - whether the carry flag was set, the return value in eax, etc.)

  return Table;

}


// This is basically the same as ReadSector(), except it reads 'logical' sectors
// (in practice, this just means the ones set by the filesystem)

// (By the way, might be a good idea to avoid reading *too much* at once, since this uses
// a lot of stack space)

realModeTable* ReadLogicalSector(uint16 NumBlocks, uint32 Address, uint32 Lba) {

  // First, we need to figure out the corresponding physical sector, like this:

  // (NumSectors is the number of physical sectors to read, whereas NumBlocks is the number
  // of logical sectors to read)

  uint32 PhysicalLba = Lba;
  uint16 Offset = 0;

  uint16 NumSectors = NumBlocks;

  if (PhysicalSectorSize < LogicalSectorSize) {

    PhysicalLba *= (LogicalSectorSize / PhysicalSectorSize);
    NumSectors *= (LogicalSectorSize / PhysicalSectorSize);

  } else if (PhysicalSectorSize > LogicalSectorSize) {

    PhysicalLba /= (PhysicalSectorSize / LogicalSectorSize);
    NumSectors = ((NumSectors - 1) / (PhysicalSectorSize / LogicalSectorSize)) + 1;

    Offset += ((Lba % (PhysicalSectorSize / LogicalSectorSize)) * LogicalSectorSize);

  }

  // Next, we'll want to create a temporary array to store the data we're reading from
  // disk. We can't just directly use ReadSector() on Address, since the size of a physical
  // sector might exceed the size of a logical sector, so instead, we do this:

  uint8 Cache[PhysicalSectorSize * NumSectors];

  // Now, we can finally use ReadSector(), and load the necessary sectors.

  realModeTable* Table = ReadSector(NumSectors, (uint32)(int)&Cache[0], PhysicalLba);

  // Finally, we can go ahead and copy the logical block/sector to the given address, and
  // return the real mode table

  Memcpy((void*)(int)Address, &Cache[Offset], (LogicalSectorSize * NumBlocks));
  return Table;

}


// A function that loads the corresponding FAT entry of a cluster, and returns the specific
// cluster value.

uint32 GetFatEntry(uint32 ClusterNum, uint32 PartitionOffset, uint32 FatOffset, bool IsFat32) {

  // If this is FAT32, then since each cluster entry is 4 bytes long (instead of 2),
  // multiply the cluster number by 2.

  if (IsFat32 == true) {
    ClusterNum *= 2;
  }

  // (Get the sector/entry offsets..)

  uint32 SectorOffset = (PartitionOffset + FatOffset + ((ClusterNum * 2) / LogicalSectorSize));
  uint16 EntryOffset = (uint16)((ClusterNum * 2) % LogicalSectorSize);

  // Next, we want to create a temporary buffer (with the same size as one sector), and then
  // load that part of the FAT onto it:

  uint8 Buffer[LogicalSectorSize];
  Memset(&Buffer[0], '\0', LogicalSectorSize);

  realModeTable* Table = ReadLogicalSector(1, (uint32)(int)&Buffer[0], SectorOffset);

  // Finally, we want to see if it succeeded, and if it did, return the corresponding entry
  // from the FAT; otherwise, return zero.

  if (hasFlag(Table->Eflags, CarryFlag)) {
    return 0;
  }

  uint32 ClusterEntry;

  if (IsFat32 == false) {
    ClusterEntry = *(uint16*)(&Buffer[EntryOffset]);
  } else {
    ClusterEntry = *(uint32*)(&Buffer[EntryOffset]);
  }

  return ClusterEntry;

}


// This function just compares a name in a

static bool FatNameIsEqual(int8 EntryName[8], int8 EntryExtension[3], char Name[8], char Extension[3], bool IsFolder) {

  // Compare the filename..

  for (int i = 0; i < 8; i++) {

    if (EntryName[i] != Name[i]) {
      return false;
    }

  }

  if (IsFolder == true) {
    return true;
  }

  // Compare the extension..

  for (int j = 0; j < 3; j++) {

    if (EntryExtension[j] != Extension[j]) {
      return false;
    }

  }

  // If all of these passed without a problem, return true.

  return true;

}


// A function that searches through a given cluster (chain) to find a specific file or
// directory.

fatDirectory FindDirectory(uint32 ClusterNum, uint8 SectorsPerCluster, uint32 PartitionOffset, uint32 FatOffset, uint32 DataOffset, char Name[8], char Extension[3], bool IsFolder, bool IsFat32) {

  // (Define limits for FAT16 and FAT32, and return if bad)

  uint32 Limit = 0xFFF6;

  if (IsFat32 == true) {
    Limit = 0x0FFFFFF6;
  }

  if (ClusterNum >= Limit) {
    goto ReturnNullEntry;
  }

  // The size of an individual FAT cluster can be pretty huge, and we don't have enough
  // memory that we can (safely) load in an entire cluster at once, so instead, we'll be loading
  // only individual (logical) sectors.

  uint32 CurrentCluster = ClusterNum;

  do {

    // Since we're going to be dividing these clusters into sectors, we're going to need
    // to load as many logical sectors as there are in a cluster.

    // In order to do this, we'll keep a buffer/cache the size of one logical sector, and
    // keep reloading it until we either reach the last cluster, or until we find what we're
    // looking for.

    uint8 ClusterCache[LogicalSectorSize];
    uint32 ClusterOffset = ((CurrentCluster - 2) * SectorsPerCluster);

    for (unsigned int SectorNum = 0; SectorNum < SectorsPerCluster; SectorNum++) {

      ReadLogicalSector(1, (uint32)ClusterCache, (PartitionOffset + DataOffset + ClusterOffset + SectorNum));

      for (unsigned int EntryNum = 0; EntryNum < (LogicalSectorSize / 32); EntryNum++) {

        // First, we need to check to see if the first byte of the entry is null (00h);
        // if it is, then that means we've already reached the last entry in the entire
        // cluster, so we can safely jump out of the loop.

        fatDirectory Entry = *(fatDirectory*)(&ClusterCache[EntryNum * 32]);

        if (Entry.Name[0] == 0x00) {
          break;
        }

        // Also, if this is a long filename entry, we can just skip forward:

        if ((Entry.Attributes & 0x0F) == 0x0F) {
          continue;
        }

        // (show data..)

        Printf("[Debug] Found a file \"", 0x03);

        for (int i = 0; i < 8; i++) {
          Putchar(Entry.Name[i], 0x07);
        }
        Putchar('.', 0x07);
        for (int j = 0; j < 3; j++) {
          Putchar(Entry.Extension[j], 0x07);
        }

        Printf("\" with attributes %xh and on address %xh\n", 0x03, Entry.Attributes, (((PartitionOffset + DataOffset + SectorNum) * LogicalSectorSize) + (EntryNum * 32)));

        // Next, we want to see if the type matches - essentially, if we're looking for a
        // file, skip through all folders, and if we're looking for a folder, skip through
        // all files.

        bool EntryIsFolder = ((Entry.Attributes & 0x10) != 0);

        if (EntryIsFolder != IsFolder) {
          continue;
        }

        // Finally, we want to compare the name and directory between the entry we're
        // looking at, and the 'target' entry that we're looking for. If it matches, then
        // we can exit out of this whole process, and return the cluster in that entry!

        if (FatNameIsEqual(Entry.Name, Entry.Extension, Name, Extension, IsFolder) == true) {
          return Entry;
        }

      }

      // Now that we've successfully checked the whole cluster, it's time to move onto
      // the next one (if applicable). The next cluster is stored in the FAT, but thankfully,
      // we already have a function that does that job for us: GetFatEntry().

      CurrentCluster = GetFatEntry(CurrentCluster, PartitionOffset, FatOffset, IsFat32);
      continue;

    }

  } while (ExceedsLimit(CurrentCluster, Limit));

  // Now, if we've gotten here, then that means we haven't found a matching entry; in that case,
  // all we need to do is to return an empty directory entry, with the highest possible limit,
  // to indicate that we didn't really find anything.

  ReturnNullEntry:

  fatDirectory NullEntry;

  NullEntry.ClusterNum_Low = 0xFFFF;
  NullEntry.ClusterNum_High = 0xFFFF;
  NullEntry.Size = 0;

  Printf("Returning null entry... initial cluster num = %x\n", 0x03, ClusterNum);

  return NullEntry;

}
