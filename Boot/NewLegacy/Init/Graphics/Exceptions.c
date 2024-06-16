// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../Memory/Memory.h"
#include "Graphics.h"

/* void Message()

   Inputs: messageType{} Type - The message type/classification (Kernel, Ok, Warning, etc.);
           char* String - The content of the message, as a single string.

   Outputs: (None)

   This function outputs a message (of a specific type/classification) to the screen.

   It's intended to communicate (important) information to the user, and it has different
   classification levels (which are defined in the messageType{} enum in Exceptions.h), which
   are shown to the user.

   For example, if you wanted to tell the user that something wrong had gone on, you could do:
   - Message(Fail, "Something wrong has happened.");

*/

void Message(messageType Type, char* String) {

  // Show the message type / 'classification level' to the user, enclosed by square
  // brackets (for example, [Kernel], [Ok], etc).

  Print("[", 0x0F);

  switch(Type) {

    case Info:
      Print("Info", 0x07);
      break;

    case Kernel:
      Print("Serra", 0x09);
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

  Print("] ", 0x0F);

  // Show the given message to the user.

  Printf("%s\n", 0x0F, String);

}


/* void __attribute__((noreturn)) Panic()

   Inputs: char* String - The error/panic message to be displayed to the user.

   Outputs: (None)

   This function essentially just shows an error message on the screen (and the address of the
   instruction/function that caused it), and then halts the system completely, forcing the
   user to restart.

   The purpose of this function is pretty much just to stop the boot process entirely if
   something goes wrong - for example, if there's an exception (like a division by 0), this
   function should be called.

*/

void __attribute__((noreturn)) Panic(char* String) {

  // Just in case, we want to disable interrupts, as they could interrupt the process

  __asm__("cli");

  // Show the error message to the user.

  Putchar('\n', 0);
  Message(Error, String);

  // Finally, tell the user that the system will halt indefinitely (and that they should
  // probably restart their computer), and then do exactly that.

  Putchar('\n', 0);

  Print("An error has occured, and this system will now halt indefinitely.\n", 0x0F);
  Print("Please restart your computer.\n", 0x0F);

  for(;;) {
    __asm__("cli; hlt");
  }

}
