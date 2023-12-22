// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

// Todo - ..I mean, everything

void Message(messageType Type, char* String) {

  // ...

  Putchar('[', 0x0F);

  switch(Type) {

    case Info:
      Print("Info", 0x07);
      break;

    case Kernel:
      Print("Kernel", 0x09);
      break;

    case Ok:
      Print("Ok", 0x0A);
      break;

    case Fail:
      Print("Fail", 0x0C);
      break;

    case Warning:
      Print("Warning", 0x0D);
      break;

    case Error:
      Print("Panic", 0x04);
      break;

    default:
      Print("Unknown", 0x08);

  }

  Putchar(']', 0x0F);
  Putchar(' ', 0x0F);

  // ...

  Print(String, 0x0F);
  Putchar('\n', 0x0F);

}


// "Translate" address

char* TranslateAddress(char* Buffer, uint32 Address) {

  if (Address > 0) {

    Memset(Buffer, '\0', 10);
    Itoa(Address, Buffer, 16);

    int AddressLength = Strlen(Buffer);

    Buffer[AddressLength + 0] = 'h';
    Buffer[AddressLength + 1] = '\0';

  } else {

    Buffer = "(Unknown)";

  }

  return Buffer;

}

// panic

void __attribute__((noreturn)) Panic(char* String, uint32 Eip) {

  // you want like, a "kernel" msg function

  // Prepare

  __asm__("cli");

  char Buffer[16];
  Memset(Buffer, '\0', 16);

  // ...

  Print("", 0x0F);
  Message(Error, String);

  // ...

  Print("Instruction pointer: ", 0x0F);
  Print(TranslateAddress(Buffer, Eip), 0x07);
  Print("\n\n", 0);

  // ...

  Message(Kernel, "An error has occured, and this system will now halt indefinitely.");
  Message(Kernel, "Please restart your computer.");

  // ...

  for(;;) {
    __asm__("hlt");
  }

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


// irq
// if ReadFromPort is false ignore Port

static char* Irqs[] = {

  "Timer (IRQ 0)",
  "PS/2 Keyboard (IRQ 1)",
  "Internal (IRQ 2)",
  "COM2 (IRQ 3)",
  "COM1 (IRQ 4)",
  "LPT2 (IRQ 5)",
  "Floppy disk (IRQ 6)",
  "LPT1 (IRQ 7)",

  "CMOS clock (IRQ 8)",
  "Peripheral port A (IRQ 9)",
  "Peripheral port B (IRQ 10)",
  "Peripheral port C (IRQ 11)",
  "PS/2 Mouse (IRQ 12)",
  "FPU/Coprocessor (IRQ 13)",
  "Primary ATA hard drive (IRQ 14)",
  "Secondary ATA hard drive (IRQ 15)"

};

void IrqHandler(uint8 Vector, uint8 Port) {

  // ...

  Message(Kernel, "An IRQ has occured.");
  Message(Info, Irqs[Vector]);

  // ... (port has to be >0x00 to do anything)

  if (Port != 0x00) {

    int a = Inb(Port);
    (void)a;

  }

  // ...

  return;

}

// ebp, ip
