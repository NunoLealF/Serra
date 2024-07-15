// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "../Memory/Memory.h"
#include "Disk.h"

// ...

realModeTable* ReadDisk(realModeTable* Table, uint8 DriveNumber, uint16 NumBlocks, uint64 Address, uint64 Offset) {

  // Create a eddDiskAddressPacket structure with the data we got.
  // (If the address we're trying to copy to doesn't fit in a 32-bit integer, then..)

  eddDiskAddressPacket DiskAddressPacket;

  DiskAddressPacket.Size = 0x18;
  DiskAddressPacket.Reserved = 0x00;

  DiskAddressPacket.Address = (uint32)(Address);
  DiskAddressPacket.NumBlocks = NumBlocks;
  DiskAddressPacket.Lba = Offset;

  if (Address >= uintmax) {
    DiskAddressPacket.Address = 0xFFFFFFFF;
    DiskAddressPacket.FlatAddress = Address;
  }

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
