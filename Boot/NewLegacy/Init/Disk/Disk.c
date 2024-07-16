// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Disk.h"

/* realModeTable* ReadDisk()

   Inputs: uint8 DriveNumber - The BIOS drive number of the drive we want to read *from*.
           uint16 NumBlocks - The number of sectors (blocks) that we want to load.
           uint64 Address - The 48-bit memory address we want to load *to*.
           uint64 Offset - The LBA (sector number) we want to start loading *from*.

   Outputs: realModeTable* - This function uses a real mode BIOS interrupt (int 13h, ah 42h) in
   order to actually load anything, so this is just the output of that; make sure to check it
   for the carry flag (and for error codes in the ah register)

   This function loads data from the disk, using the BIOS function (int 13h, ah 42h);
   specifically, it loads [NumSectors] sectors starting from the LBA [Offset] (on the disk
   indicated by [DriveNumber]) to the memory address at [Address].

   Since it isn't really necessary, this function doesn't support 64-bit memory addresses, but
   the disk address packet format used by this BIOS function does support it (although it's
   unclear whether many BIOSes support it at all).

*/

realModeTable* ReadDisk(uint8 DriveNumber, uint16 NumBlocks, uint64 Address, uint64 Offset) {

  // Create a eddDiskAddressPacket structure with the data we got.
  // (If the address we're trying to copy to doesn't fit in a 32-bit integer, then..)

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




// A function that loads the corresponding FAT entry of a cluster, and returns the specific
// cluster value.

uint16 GetFat16_Entry(uint8 DriveNumber, uint16 ClusterNum, uint16 Edd_BytesPerSector, uint16 Fat_BytesPerSector, uint32 PartitionOffset, uint32 FatOffset) {

  // First, we want to figure out the sector/LBA of the FAT entry that we're looking for,
  // along with the offset.

  // That being said, because sector sizes often vary between what the BPB uses and what our
  // system (or, at least, the EDD functions) use, we'll be converting between sector sizes.

  uint32 SectorOffset = (PartitionOffset + FatOffset + ((ClusterNum * 2) / Fat_BytesPerSector));
  uint16 EntryOffset = (ClusterNum * 2);

  if (Edd_BytesPerSector < Fat_BytesPerSector) {

    SectorOffset *= (Fat_BytesPerSector / Edd_BytesPerSector);
    SectorOffset += ((EntryOffset % Fat_BytesPerSector) / Edd_BytesPerSector);

  } else if (Edd_BytesPerSector > Fat_BytesPerSector) {

    SectorOffset /= (Edd_BytesPerSector / Fat_BytesPerSector);

  }

  EntryOffset %= Edd_BytesPerSector;

  // Next, we want to create a temporary buffer (with the same size as one sector), and then
  // load that part of the FAT onto it:

  uint8 FatBuffer[Edd_BytesPerSector];
  Memset(&FatBuffer[0], '\0', Edd_BytesPerSector);

  realModeTable* Table = ReadDisk(DriveNumber, 1, (uint32)FatBuffer, SectorOffset);

  // Finally, we want to see if it succeeded, and if it did, return the corresponding entry
  // from the FAT; otherwise, return zero.

  if (hasFlag(Table->Eflags, CarryFlag)) {
    return 0;
  }

  uint16 ClusterEntry = *(uint16*)(&FatBuffer[EntryOffset]);
  return ClusterEntry;

}


// This is essentially the same, but for FAT32

uint32 GetFat32_Entry(uint8 DriveNumber, uint32 ClusterNum, uint16 Edd_BytesPerSector, uint16 Fat_BytesPerSector, uint32 PartitionOffset, uint32 FatOffset) {

  // First, we want to figure out the sector/LBA of the FAT entry that we're looking for,
  // along with the offset.

  // That being said, because sector sizes often vary between what the BPB uses and what our
  // system (or, at least, the EDD functions) use, we'll be converting between sector sizes.

  uint32 SectorOffset = (PartitionOffset + FatOffset + ((ClusterNum * 4) / Fat_BytesPerSector));
  uint16 EntryOffset = (ClusterNum * 4);

  if (Edd_BytesPerSector < Fat_BytesPerSector) {

    SectorOffset *= (Fat_BytesPerSector / Edd_BytesPerSector);
    SectorOffset += ((EntryOffset % Fat_BytesPerSector) / Edd_BytesPerSector);

  } else if (Edd_BytesPerSector > Fat_BytesPerSector) {

    SectorOffset /= (Edd_BytesPerSector / Fat_BytesPerSector);

  }

  EntryOffset %= Edd_BytesPerSector;

  // Next, we want to create a temporary buffer (with the same size as one sector), and then
  // load that part of the FAT onto it:

  uint8 FatBuffer[Edd_BytesPerSector];
  Memset(&FatBuffer[0], '\0', Edd_BytesPerSector);

  realModeTable* Table = ReadDisk(DriveNumber, 1, (uint32)FatBuffer, SectorOffset);

  // Finally, we want to see if it succeeded, and if it did, return the corresponding entry
  // from the FAT; otherwise, return zero.

  if (hasFlag(Table->Eflags, CarryFlag)) {
    return 0;
  }

  uint32 ClusterEntry = *(uint32*)(&FatBuffer[EntryOffset]);
  return ClusterEntry;

}
