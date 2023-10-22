// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"

// Todo - uhhh, probably everything at this point.
// IDT should be at D000h.
// IDTR should be (2048-1), D000h.

// We should probably try to initialize it as soon as possible, so that we can deal with
// exceptions.

// Also, we need to implement it in Rm.asm when we're coming back from real mode.

// ...

typedef struct {

  uint16 Size;
  uint32 Offset;

} __attribute__((packed)) descriptorTable;

/// ...

typedef struct {

  uint16 OffsetLow;
  uint16 Selector;
  uint8 Reserved;
  uint8 Flags;
  uint16 OffsetHigh;

} __attribute__((packed)) idtEntry;

// ...

extern void IsrNoErrorStub(void);
extern void IsrFaultStub(void);
extern void IsrAbortStub(void);

// ...

void LoadIdt(descriptorTable* IdtDescriptor) {

  __asm__("lidt %0" : : "m"(*IdtDescriptor));

}

// ...

void MakeIdtEntry(descriptorTable* IdtDescriptor, uint16 EntryNum, uint32 Offset, uint16 Selector, uint8 Gate, uint8 Dpl) {

  // IDT location is IdtDescriptor->Offset
  // We add (EntryNum*8) (each entry is 8 bytes long) to get the actual memory address

  uint32 EntryLocation = (IdtDescriptor->Offset) + (EntryNum * 8);
  idtEntry* Entry = (idtEntry*)EntryLocation;

  // Fill in values

  Entry->OffsetLow = (uint16)(Offset & 0xFFFF);
  Entry->OffsetHigh = (uint16)((Offset >> 16) & 0xFFFF);

  Entry->Selector = Selector;

  Entry->Flags = (Gate & 0x0F) + ((Dpl & 0x03) << 5) + (1 << 7);
  Entry->Reserved = 0;

}
