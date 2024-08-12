// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../../Shared/Rm/Rm.h"
#include "Graphics.h"

// A few things to keep in mind:
// -> *All VESA functions return 4Fh in AL if they are supported, and use AH as a status flag*
// -> The VESA/VBE specification is here: https://pdos.csail.mit.edu/6.828/2012/readings/hardware/vbe3.pdf


/*

(some important functions)

00h -> get VESA/VBE info, along with all video modes (query this first)
01h -> get mode info
02h -> set a video mode
03h -> get current mode

04h (?) -> save/restore state? hmm



*/



// (far pointers, with segment and offset; these are used a *lot* by VBE)

typedef struct {

  uint16 Segment;
  uint16 Offset;

} __attribute__((packed)) farPtr;

#define convertFarPtr(Address) ((Ptr.Segment << 4) + Ptr.Offset)



//
// (ax = 4F00h, es:di = location) is the most important one
// This is what it returns:

typedef struct {

  // (VBE 1.x)

  uint32 Signature; // This should just be 'VESA'
  uint16 Version; // Usually the high 8 bits are the overall version number, so, for example, VBE 1.1 is 1010h, and VBE 3.0 is 3000h

  farPtr OemStringPtr; // Far pointer to a null-terminated string
  uint32 Capabilities; // This is a bitfield (defined in the VESA spec)
  farPtr VideoModeListPtr; // Far pointer to a list of 16-byte mode numbers, where the last entry is FFFFh (to mark the end of the list)
  uint16 NumBlocks; // The number of memory blocks (64 KiB each)

  // (VBE 2.x and higher)

  struct __OemInfo {

    uint16 VbeRevision;
    farPtr VendorNamePtr;
    farPtr ProductNamePtr;
    farPtr ProductRevPtr;

  } __attribute__((packed)) OemInfo;

  // (Reserved for future use; the size of all of the above is 34 bytes, so 512-32 = 478)

  uint8 Reserved[478];

} __attribute__((packed)) vbeInfoBlock;





// Something to check if VBE is supported

bool VbeIsSupported(void) {

  // Todo...

}
