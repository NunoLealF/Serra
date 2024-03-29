// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"
#include "../Graphics/Graphics.h"
#include "../Exceptions.h"

/* (typedef) struct __attribute__((packed)) descriptorTable{}

   Location: (User-defined)

   Elements: uint16 Size - The size of the GDT/IDT, in bytes.
             uint32 Offset - The location of the GDT/IDT, expressed as a 32-bit memory address.

   This structure defines an important structure for both the GDT (General Descriptor Table)
   and the IDT (Interrupt Descriptor Table) - its descriptor.

   It contains two values - the size of the GDT/IDT, and the 'offset' (or, more accurately,
   the location) of the GDT/IDT, expressed as a memory address. As we're in 32-bit mode right
   now, that memory address is also 32 bits wide.

*/

typedef struct {

  uint16 Size;
  uint32 Offset;

} __attribute__((packed)) descriptorTable;


/* (typedef) struct __attribute__((packed)) idtEntry{}

   Location: (D000h + (vNum * 8))

   Elements: uint16 OffsetLow - ...
             uint16 Selector - ...
             uint8 Reserved - ...
             uint8 Flags - ...
             uint16 OffsetHigh - ...

   This structure defines an IDT (gate) entry. It contains four main values:
   - The offset, which indicates the location of the ISR assigned to that IDT entry;
   - The segment selector, which must point to a valid code segment in our GDT;
   - The reserved area, which isn't really used for anything;
   - The flags, which contain important information about the nature of the IDT entry.

   For more information, please refer to volume 3 of the Intel(R) 64 and IA-32 Architectures
   Software Developer's Manual.

*/

typedef struct {

  uint16 OffsetLow;
  uint16 Selector;
  uint8 Reserved;
  uint8 Flags;
  uint16 OffsetHigh;

} __attribute__((packed)) idtEntry;


/* void LoadIdt()

   Inputs: descriptorTable* IdtDescriptor - The IDT descriptor that we want to load.
   Outputs: (None)

   This function takes the role of one simple, but very important job - it loads an IDT.
   Specifically, it's a wrapper around the 'lidt' instruction, that takes the address to an
   IDT descriptor table.

   Essentially speaking, the purpose of this function is to load an IDT.

*/

void LoadIdt(descriptorTable* IdtDescriptor) {

  __asm__("lidt %0" : : "m"(*IdtDescriptor));

}


/* void MakeIdtEntry()

   Inputs: descriptorTable* IdtDescriptor - The IDT descriptor that we want to use.
           uint16 EntryNum - The entry/vector number of the entry we want to make.
           uint32 Offset - The location of the entry point of the ISR of our entry.
           uint16 Selector - The segment selector of the IDT entry we want to make.
           uint8 Gate - The gate type of the IDT entry we want to make.
           uint8 Dpl - The privilege level(s) allowed to access the IDT entry we want to make.

   Outputs: (None)

   This function writes an IDT entry to memory using the given information. With the exception
   of IdtDescriptor, each parameter (roughly) corresponds to a value in idtEntry{} - for more
   details, check the documentation on that function, or consult section 6.11 / volume 3 of the
   Intel(R) 64 and IA-32 Architectures Software Developer's Manual.

   As an example, if you wanted to make an IDT entry with the following characteristics:

   - Using an IDT descriptor named TestIdt (descriptorTable* TestIdt);
   - For the entry/vector number 04h (which means it would be triggered with "int $0x04");
   - Corresponds to an ISR with an entry point at CAFEBABEh in memory;
   - Uses a segment selector of 08h;
   - Which functions as a 32-bit trap gate (0Fh);
   - That can only be accessed from CPU privilege level 0 (00h);

   You would do the following:

   -> MakeIdtEntry(TestIdt, 0x04, 0xCAFEBABE, 0x08, 0x0F, 0x00);

*/

void MakeIdtEntry(descriptorTable* IdtDescriptor, uint16 EntryNum, uint32 Offset, uint16 Selector, uint8 Gate, uint8 Dpl) {

  // The location of our IDT is at IdtDescriptor->Offset, and the location of our entry should
  // be at (IdtDescriptor->Offset) + (EntryNum * 8), as each entry occupies 8 bytes.

  uint32 EntryLocation = (IdtDescriptor->Offset) + (EntryNum * 8);

  // Now, we want to make an entry at that memory location (using the idtEntry{} struct), and
  // fill it in with the proper values.

  idtEntry* Entry = (idtEntry*)EntryLocation;

  Entry->OffsetLow = (uint16)(Offset & 0xFFFF);
  Entry->OffsetHigh = (uint16)((Offset >> 16) & 0xFFFF);

  Entry->Selector = Selector;

  Entry->Flags = (Gate & 0x0F) + ((Dpl & 0x03) << 5) + (1 << 7);
  Entry->Reserved = 0;

}


// This is an array of strings that essentially indicates which one belongs to each ISR (not
// IRQ - that's defined in a separate file).

// For example, if you received an ISR 7 ("int $0x07"), you could look up Exceptions[7] and
// figure out that it corresponds to "Device not available (ISR 7)".

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


/* void IsrFault()

   Inputs: uint8 Vector - The specific vector number of our interrupt.
           uint32 Eip - The address of the instruction that caused the exception.

   Outputs: (None, except a warning message on the screen)

   This function serves as an interrupt handler for an exception. It's called by any ISR which
   uses the 'IsrFaultStub' macro in Isr.s, and it takes two parameters - the specific vector
   number of the interrupt, and the instruction pointer pushed onto the stack at the time of
   the exception (which represents the address of the faulty instruction).

   Using the Message() function from Exceptions.c, it shows to the user that an exception has
   occured, and shows information about which exception occured (using the Exceptions[] list),
   and about what caused it (by showing Eip).

   This interrupt handler should only be used by ISRs that correspond to a fault that can be
   recovered from, and that *doesn*'t contain an error code - if the latter doesn't apply, use
   IsrFaultWithError() instead.

*/

void IsrFault(uint8 Vector, uint32 Eip) {

  // Tell the user that an exception has happened, and using the Exceptions[] list, show which
  // one did.

  Putchar('\n', 0);

  Message(Warning, "An exception has occured.");
  Message(Info, Exceptions[Vector]);

  // Show the user the address of the faulty instruction.

  Print("EIP (Instruction pointer): ", 0x0F);
  Printf("%xh\n", 0x07, Eip);

}


/* void IsrFaultWithError()

   Inputs: uint8 Vector - The specific vector number of our interrupt.
           uint32 Eip - The address of the instruction that caused the exception.
           uint32 Error - The error code pushed onto the stack by the exception.

   Outputs: (None, except a warning message on the screen)

   This function serves as an interrupt handler for an exception. It's called by any ISR which
   uses the 'IsrFaultStubWithError' macro in Isr.s, and it takes three parameters - the specific
   vector number of the interrupt, the instruction pointer pushed onto the stack at the time of
   the exception, and the error code (also pushed onto the stack).

   Using the Message() function from Exceptions.c, it shows to the user that an exception has
   occured, and shows information about which exception occured (using the Exceptions[] list),
   and about what caused it (by showing Eip and Error).

   This interrupt handler should only be used by ISRs that correspond to a fault that can be
   recovered from, and that *does* contain an error code - if the latter doesn't apply, use
   IsrFault() instead.

*/

void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error) {

  // Tell the user that an exception has occured, and using the Exceptions[] list, show which
  // one did.

  Putchar('\n', 0);

  Message(Warning, "An exception has occured.");
  Message(Info, Exceptions[Vector]);

  // Show the instruction pointer of the faulty instruction *and* the error code that was
  // pushed onto the stack by the exception to the user.

  Print("Error code: ", 0x0F);
  Printf("%xh\n", 0x07, Error);

  Print("EIP (Instruction pointer): ", 0x0F);
  Printf("%xh\n", 0x07, Eip);

}


/* void IsrAbort()

   Inputs: uint8 Vector - The specific vector number of our interrupt.
           uint32 Eip - The address of the instruction that caused the exception.

   Outputs: (None, except a warning message on the screen)

   This function serves as an interrupt handler for an unrecoverable exception/fault. It's called
   by any ISR which uses the 'IsrAbortStub' macro in Isr.s, and it takes two parameters - the
   specific vector number of the interrupt, and the instruction pointer pushed onto the stack at
   the time of the exception (which represents the address of the faulty instruction).

   Using the Message() and Panic() functions from Exceptions.c, it shows to the user that an
   exception has occured, shows information about which exception occured (using the Exceptions[]
   list), and then halts the system (this occurs inside of Panic()).

   This interrupt handler should only be used by ISRs that correspond to a fault that cannot be
   recovered from, whether or not it contains an error code - if the former doesn't apply, use
   any other Isr...() function instead.

*/

void IsrAbort(uint8 Vector, uint32 Eip) {

  // Tell the user that an exception has occured, and using the Exceptions[] list, show which
  // one did, before halting the system (this happens inside of Panic()).

  Putchar('\n', 0);
  Message(Warning, "An exception has occured.");

  Panic(Exceptions[Vector], Eip);

}


/* void IsrLog()

   Inputs: uint8 Vector - The specific vector number of our interrupt.
   Outputs: (None, except an information message on the screen)

   This function serves as a generic interrupt handler / ISR for any interrupt that isn't
   necessarily problematic (like an NMI interrupt).

   Using the Message() function from Exception.c, it shows to the user that an interrupt has
   occured, shows information about which one has (using the Exceptions[] list from earlier),
   and then returns.

   This function is intended to be called from the 'IsrLogStub' macro in Isr.s, and it should
   only be used for interrupts that aren't considered to be faults/exceptions, that don't push
   an error code onto the stack, and that don't require some sort of action before returning
   (like with certain IRQs).

*/

void IsrLog(uint8 Vector) {

  // Tell the user that an interrupt has occured, and using the Exceptions[] list, show which
  // one did.

  Putchar('\n', 0);
  Message(Kernel, "An interrupt has occured.");

  Message(Info, Exceptions[Vector]);

}
