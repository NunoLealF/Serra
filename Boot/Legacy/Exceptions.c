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


// panic

void __attribute__((noreturn)) Panic(char* String, uint32 Eip) {

  // you want like, a "kernel" msg function

  // Prepare

  __asm__("cli");

  char Buffer[8];
  char* EipString = Itoa(Eip, Buffer, 16);

  // ...

  Print("", 0x0F);
  Message(Error, String);

  // ...

  Print("EIP: ", 0x0F);
  Print(EipString, 0x07);
  Print("h\n\n", 0x07);

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

  Message(Info, Exceptions[Vector]);

  // ...

  char Buffer[8];
  char* EipString = Itoa(Eip, Buffer, 16);

  Print("EIP: ", 0x0F);
  Print(EipString, 0x07);
  Print("h\n", 0x07);

  // ...

  Message(Warning, "A fault has occured.");

  return;

}


void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error) {

  // ...

  Message(Info, Exceptions[Vector]);

  // ...

  char Buffer[8];
  char* ErrorString = Itoa(Error, Buffer, 16);

  Print("Error code: ", 0x0F);
  Print(ErrorString, 0x07);
  Print("h\n", 0x07);

  // ...

  Memset(Buffer, 0, 8);
  char* EipString = Itoa(Eip, Buffer, 16);

  Print("EIP: ", 0x0F);
  Print(EipString, 0x07);
  Print("h\n", 0x07);

  // ...

  Message(Warning, "A fault has occured.");

  return;

}


void IsrAbort(uint8 Vector, uint32 Eip) {

  // ...

  Message(Info, Exceptions[Vector]);

  // ...

  Panic("An exception has occured.", Eip);
  return;

}


void IsrLog(uint8 Vector, uint32 Eip) {

  // ...

  Message(Info, "An interrupt has occured.");

  // ...

  Message(Info, Exceptions[Vector]);

  // ...

  Message(Info, "An interrupt has occured.");

  return;

}


// irq
// if ReadFromPort is false ignore Port

void IrqHandler(uint8 Vector, bool ReadFromPort, uint8 Port) {

  // ...

  Message(Info, "An IRQ has occured.");

  // ...

  if (ReadFromPort == true) {

    (void)Inb(Port);

  }

  // ...

  return;

}

// ebp, ip
