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
           uint32 Address - The memory address we want to load *to*.
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

realModeTable* ReadDisk(uint8 DriveNumber, uint16 NumBlocks, uint32 Address, uint64 Offset) {

  // Create a eddDiskAddressPacket structure with the data we got.
  // (If the address we're trying to copy to doesn't fit in a 32-bit integer, then..)

  eddDiskAddressPacket DiskAddressPacket;

  DiskAddressPacket.Size = 0x18;
  DiskAddressPacket.Reserved = 0x00;

  DiskAddressPacket.Address = Address;
  DiskAddressPacket.FlatAddress = Address;
  DiskAddressPacket.NumBlocks = NumBlocks;
  DiskAddressPacket.Lba = Offset;

  // Now, we want to prepare to actually read from disk - specifically, we'll be using the
  // BIOS function (int 13h, ah 42h).

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
