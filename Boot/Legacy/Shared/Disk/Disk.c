// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Rm/Rm.h"
#include "Disk.h"

// Global functions and variables. (These are also defined in Disk.h, but just in case..)

uint8 DriveNumber;

uint16 LogicalSectorSize;
uint16 PhysicalSectorSize;

extern void Memcpy(void* Destination, void* Source, uint32 Size);
extern void Memset(void* Buffer, uint8 Character, uint32 Size);


/* realModeTable* ReadSector()

   Inputs: uint16 NumBlocks - The number of physical sectors (blocks) that we want to load.
           uint32 Address - The 32-bit memory address we want to load *to*.
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


/* realModeTable* ReadLogicalSector()

   Inputs: uint16 NumBlocks - The number of logical sectors (blocks) that we want to load.
           uint32 Address - The 32-bit memory address we want to load *to*.
           uint64 Offset - The LBA (sector number) we want to start loading *from*.

   Outputs: realModeTable* - This function uses a real mode BIOS interrupt (int 13h, ah 42h)
   in order to actually load anything, so this is just the output of that; make sure to check
   it for the carry flag (and for error codes in the ah register)

   This function serves essentially the same role as ReadSector(), but it reads a *logical*
   sector, which can be useful for some filesystem operations (for example, reading 4096-byte
   logical sectors on a system with 512-byte physical sectors).

   It uses the global variables LogicalSectorSize (the logical sector size of the volume,
   usually expressed in the BPB) and PhysicalSectorSize (the actual sector size being used
   by the system), so those two variables must be set.

   Otherwise, it runs in the same way as ReadSector(); *just be careful to avoid exceeding
   the stack size, since this function allocates a buffer/cache.*

*/

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


/* uint32 GetFatEntry()

   Inputs: uint32 ClusterNum - The cluster number that we want to check the FAT for.

           uint32 PartitionOffset - The first LBA of the current partition (relative to the
           start of the disk itself; this is the same as the number of hidden sectors).

           uint32 FatOffset - The first LBA of this partition's FAT (relative to the start of
           the partition itself; this is the same as the number of reserved sectors).

           bool IsFat32 - Whether this partition is FAT16 (false) or FAT32 (true).

   Outputs: uint32 - The cluster number indicated in the FAT (file allocation table).

   This function reads a given partition's FAT (file allocation table) in order to find the
   next cluster indicated by the FAT, which is useful for following cluster chains.

   Every cluster is part of a cluster chain, which is basically a singly-linked list of
   clusters, and the data structure that contains the information about these cluster chains
   is called the File Allocation Table (FAT).

   The role of this function is to read the FAT to find the next cluster for a given cluster;
   for example, if there's a cluster chain like 1234h -> 5678h, then that means the FAT entry
   for the cluster 1234h is 5678h, and this function helps find that.

*/

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


/* static bool FatNameIsEqual()

   Inputs: int8 EntryName[8] - The name specified in the entry we want to compare.
           int8 EntryExtension[3] - The extension specified in the entry we want to compare.
           char Name[8] - The name we want to compare the entry to.
           char Extension[3] - The extension we want to compare the entry to.
           bool IsFolder - Whether the entry we're trying to find is a folder or not.

   Outputs: bool - Whether the entry matches the name and extension we're looking for.

   This function has one relatively simple role - it compares the data within a given entry
   with a target name and extension, and returns true if the entry matches the target (and
   false otherwise).

*/

static bool FatNameIsEqual(int8 EntryName[8], int8 EntryExtension[3], char Name[8], char Extension[3], bool IsFolder) {

  // Compare the name..

  for (int i = 0; i < 8; i++) {

    if (EntryName[i] != Name[i]) {
      return false;
    }

  }

  if (IsFolder == true) {
    return true;
  }

  // Compare the extension.. (this is only necessary if IsFolder is false)

  for (int j = 0; j < 3; j++) {

    if (EntryExtension[j] != Extension[j]) {
      return false;
    }

  }

  // If all of these passed without a problem, return true.

  return true;

}


/* fatDirectory FindDirectory()

   Inputs: uint32 ClusterNum - The first cluster number of the folder we want to search in.

           uint8 SectorsPerCluster - The amount of sectors per cluster (specified by the BPB).

           uint32 PartitionOffset - The first LBA of the current partition (relative to the
           start of the disk itself; this is the same as the number of hidden sectors).

           uint32 FatOffset - The first LBA of this partition's FAT (relative to the start of
           the partition itself; this is the same as the number of reserved sectors).

           uint32 DataOffset - The first LBA of this partition's data section (relative to
           the start of the partition itself).

           char Name[8] - The name of the directory that we're trying to find.

           char Extension[3] - The extension of the directory that we're trying to find, if
           applicable (in the case of folders, leave this blank).

           bool IsFolder - Whether our target directory is a folder or not.

           bool IsFat32 - Whether this partition is FAT16 (false) or FAT32 (true).

   Outputs: fatDirectory - If we did find the directory, then this corresponds to a copy of
   its entry; otherwise, it corresponds to an empty entry with the limit set to FFFFFFFFh, and
   the size set to 0 (indicating that the search failed).

   This function has one relatively simple (yet complicated) role - it tries to find a
   directory (which can be a file or a folder) within a given folder, within a FAT16 or a
   FAT32 file system.

   The process to do so is well documented within the function itself, but essentially, it
   passes through the cluster chain (starting at ClusterNum), scanning all the entries within
   each cluster for the target directory, returning it if it's found.

   Keep in mind that this doesn't support long file names, and that all names and extensions
   must be uppercase and padded with spaces (for example, "Serra.bin" should turn into the
   name "SERRA   " and extension "BIN" respectively).

*/

fatDirectory FindDirectory(uint32 ClusterNum, uint8 SectorsPerCluster, uint32 PartitionOffset, uint32 FatOffset, uint32 DataOffset, char Name[8], char Extension[3], bool IsFolder, bool IsFat32) {

  // (Define limits for FAT16 and FAT32)

  uint32 Limit = 0xFFF6;

  if (IsFat32 == true) {
    Limit = 0x0FFFFFF6;
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

  } while (!ExceedsLimit(CurrentCluster, Limit));

  // Now, if we've gotten here, then that means we haven't found a matching entry; in that case,
  // all we need to do is to return an empty directory entry, with the highest possible limit,
  // to indicate that we didn't really find anything.

  fatDirectory NullEntry;

  NullEntry.ClusterNum_Low = 0xFFFF;
  NullEntry.ClusterNum_High = 0xFFFF;
  NullEntry.Size = 0;

  return NullEntry;

}


/* void ReadFile()

   Inputs: void* Address - The address we want to read the file *to*.

           fatDirectory Entry - The (directory) entry of the file we want to read *from*.

           uint8 SectorsPerCluster - The amount of sectors per cluster (specified by the BPB).

           uint32 PartitionOffset - The first LBA of the current partition (relative to the
           start of the disk itself; this is the same as the number of hidden sectors).

           uint32 FatOffset - The first LBA of this partition's FAT (relative to the start of
           the partition itself; this is the same as the number of reserved sectors).

           uint32 DataOffset - The first LBA of this partition's data section (relative to
           the start of the partition itself).

           bool IsFat32 - Whether this partition is FAT16 (false) or FAT32 (true).

   Outputs: bool - Whether the operation was successful (true) or not (false).

   This function reads the contents of a file from disk to memory, given an address to load
   it to, and the directory entry of the file we want to read from (as returned by
   FindDirectory()).

   Despite being more inefficient, this function saves on memory space by only loading one
   logical sector at a time, since the individual cluster size may exceed the remaining space
   on the stack.

*/

bool ReadFile(void* Address, fatDirectory Entry, uint8 SectorsPerCluster, uint32 PartitionOffset, uint32 FatOffset, uint32 DataOffset, bool IsFat32) {

  // (Define limits for FAT16 and FAT32)

  uint32 Limit = 0xFFF6;

  if (IsFat32 == true) {
    Limit = 0x0FFFFFF6;
  }

  // Before we do anything, we want to get two variables from the entry we were given - the
  // initial starting cluster, and the size of the file we're reading.

  uint32 ClusterNum = GetDirectoryCluster(Entry);
  uint32 Size = Entry.Size;

  // Next, we want to actually start reading from the file, until we hit either the end of
  // the cluster chain, or the end of the file. We'll be keeping track of the offset and the
  // number of bytes read using the Offset variable.

  uint32 Offset = 0;

  while (!ExceedsLimit(ClusterNum, Limit)) {

    // We'll only be reading one (logical) sector at a time, in order to conserve memory
    // space, as the cluster size can sometimes be bigger than the stack size

    uint32 ClusterOffset = ((ClusterNum - 2) * SectorsPerCluster);

    // For every sector in the current cluster..

    for (unsigned int SectorNum = 0; SectorNum < SectorsPerCluster; SectorNum++) {

      // Read from the disk, and see if it failed

      realModeTable* Table = ReadLogicalSector(1, (uint32)((int)Address + Offset), (PartitionOffset + DataOffset + ClusterOffset + SectorNum));
      Offset += LogicalSectorSize;

      if (hasFlag(Table->Eflags, CarryFlag)) {
        return false;
      }

      // If we've already reached the end of the file, no need to continue, just end the
      // function immediately.

      if (Offset >= Size) {
        return true;
      }

    }

    // Finally, if we've already finished this cluster, then continue to the next cluster
    // in the chain (if it exceeds the limit, the condition of the while loop will end things
    // here).

    ClusterNum = GetFatEntry(ClusterNum, PartitionOffset, FatOffset, IsFat32);

    if (ClusterNum == 0) {
      return false;
    }

  }

  // After everything, we can just return true (to indicate that everything went okay)

  return true;

}
