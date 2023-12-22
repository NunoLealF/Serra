// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"

#include "Graphics/Graphics.h"
#include "Memory/Memory.h"
#include "Int/Int.h"
#include "Exceptions.h"

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
