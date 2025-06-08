// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Firmware/Firmware.h"
#include "../../Memory/Memory.h"
#include "../Disk.h"
#include "Efi.h"

// (TODO - Include a function to interface with EFI's disk read protocols;
// maybe one for fs as well?)

// I'll go with the block protocol, since it's closer to what the int13h
// driver uses (sectors, not bytes), and has some extra info; they're also
// interchangeable with efiDiskIoProtocol



// I'm going to need a function to essentially:

// -> (1) Find the number of efiBlockIoProtocol handles, by calling
// gBS->LocateHandle() with an empty buffer

// -> (2) Allocate a buffer with the necessary size for that, using
// the memory management subsystem

// -> (3) Call `LocateHandle` again, this time actually

// -> Find all efiBlockIoProtocol handles, using
