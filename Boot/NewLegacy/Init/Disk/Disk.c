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
           uint64 Address - The 48-bit memory address we want to load *to*.
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

realModeTable* ReadSector(uint16 NumBlocks, uint64 Address, uint64 Offset) {

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

  Table->Ds = ((unsigned int)(&DiskAddressPacket) >> 4);
  Table->Si = ((unsigned int)(&DiskAddressPacket) & 0x0F);

  Table->Int = 0x13;
  RealMode();

  // Finally, we want to return our real mode table (which should be able to indicate a variety
  // of things - whether the carry flag was set, the return value in eax, etc.)

  return Table;

}


// This is basically the same as ReadSector(), except it reads 'logical' sectors
// (in practice, this just means the ones set by the filesystem)

realModeTable* ReadLogicalSector(uint16 NumBlocks, uint64 Address, uint32 Lba) {

  // First, we need to figure out the corresponding physical sector, like this:

  uint32 PhysicalLba = Lba;
  uint16 Offset = 0;

  if (PhysicalSectorSize < LogicalSectorSize) {

    PhysicalLba *= (LogicalSectorSize / PhysicalSectorSize);
    NumBlocks *= (LogicalSectorSize / PhysicalSectorSize);

  } else if (PhysicalSectorSize > LogicalSectorSize) {

    PhysicalLba /= (PhysicalSectorSize / LogicalSectorSize);
    Offset += ((Lba % PhysicalSectorSize) * LogicalSectorSize);

  }

  // Next, we'll want to create a temporary array to store the data we're reading from
  // disk. We can't just directly use ReadSector() on Address, since the size of a physical
  // sector might exceed the size of a logical sector, so instead, we do this:

  uint16 Size = LogicalSectorSize;

  if (PhysicalSectorSize > LogicalSectorSize) {
    Size = PhysicalSectorSize;
  }

  uint8 Cache[Size];

  // Now, we can finally use ReadSector(), and load the necessary sectors.

  realModeTable* Table = ReadSector(NumBlocks, (uint64)Cache, PhysicalLba);

  // Finally, we can go ahead and copy the logical block/sector to the given address, and
  // return the real mode table

  Memcpy((void*)Address, (void*)&Cache[Offset], LogicalSectorSize);
  return Table;

}


// A function that loads the corresponding FAT entry of a cluster, and returns the specific
// cluster value.

uint32 GetFatEntry(uint16 ClusterNum, uint32 PartitionOffset, uint32 FatOffset, bool IsFat32) {

  // If this is FAT32, then since each cluster entry is 4 bytes long (instead of 2),
  // multiply the cluster number by 2.

  if (IsFat32 == true) {
    ClusterNum *= 2;
  }

  // (Get the sector/entry offsets..)

  uint32 SectorOffset = (PartitionOffset + FatOffset + ((ClusterNum * 2) / LogicalSectorSize));
  uint16 EntryOffset = ((ClusterNum * 2) % LogicalSectorSize);

  // Next, we want to create a temporary buffer (with the same size as one sector), and then
  // load that part of the FAT onto it:

  uint8 Buffer[LogicalSectorSize];
  Memset(&Buffer[0], '\0', LogicalSectorSize);

  realModeTable* Table = ReadLogicalSector(1, (uint32)Buffer, SectorOffset);

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
