// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"
#include "../Exceptions.h"

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

// - ISR/IRQ handlers (code, [eax], ebp, eip)

// exceptions
// using the names from intel's manual (except for the last 3, which come from AMD's)

static char* Exceptions[] = {

  "Divide error (ISR 0)",

  "Debug exception (ISR 1)",
  "NMI interrupt (ISR 2)",
  "Breakpoint (ISR 3)",
  "Overflow (ISR 4)",
  "Bound range exceeded (ISR 5)",

  "Invalid opcode (ISR 6)",
  "Device not available (ISR 7)",
  "Double fault (ISR 8)",
  "Coprocessor segment overrun (ISR 9)",
  "Invalid TSS (ISR 10)",
  "Segment not present (ISR 11)",
  "Stack-segment fault (ISR 12)",
  "General protection fault (ISR 13)",
  "Page fault (ISR 14)",

  "Unknown (ISR 15)",
  "x87 floating-point error (ISR 16)",
  "Alignment check (ISR 17)",
  "Machine check (ISR 18)",
  "SIMD floating-point exception (ISR 19)",

  "Virtualization exception (ISR 20)",
  "Control protection exception (ISR 21)",

  "Unknown (ISR 22)",
  "Unknown (ISR 23)",
  "Unknown (ISR 24)",
  "Unknown (ISR 25)",
  "Unknown (ISR 26)",
  "Unknown (ISR 27)",

  "Hypervisor injection exception (ISR 28)",
  "VMM communication exception (ISR 29)",
  "Security exception (ISR 30)",
  "Unknown (ISR 31)"

};

// A -> fault
// B -> fault with error
// C -> abort (with or without error)
// D -> note/log
// Irq -> irq handler

void IsrFault(uint8 Vector, uint32 Eip) {

  // ...

  Message(Warning, "An exception has occured.");
  Message(Info, Exceptions[Vector]);

  // ...

  char Buffer[16];
  Memset(Buffer, '\0', 16);

  Print("Instruction pointer: ", 0x0F);
  Print(TranslateAddress(Buffer, Eip), 0x07);
  Putchar('\n', 0);

  // ...

  return;

}


void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error) {

  // ...

  Message(Warning, "An exception has occured.");
  Message(Info, Exceptions[Vector]);

  // ...

  char Buffer[16];
  Memset(Buffer, '\0', 16);

  Print("Error code: ", 0x0F);
  Print(TranslateAddress(Buffer, Error), 0x07);

  Print("\nInstruction pointer: ", 0x0F);
  Print(TranslateAddress(Buffer, Eip), 0x07);

  Putchar('\n', 0);

  // ...

  return;

}


void IsrAbort(uint8 Vector, uint32 Eip) {

  // ...

  Message(Warning, "An exception has occured.");
  Panic(Exceptions[Vector], Eip);

  // ...

  return;

}


void IsrLog(uint8 Vector) {

  // ...

  Message(Kernel, "An interrupt has occured.");
  Message(Info, Exceptions[Vector]);

  // ...

  return;

}
