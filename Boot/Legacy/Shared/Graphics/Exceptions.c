// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"
#include "Graphics.h"

/* void Message()

   Inputs: messageType{} Type - The message type/classification (Boot, Ok, Warning, etc.);
           char* String - The content of the message, as a single string.
           (...) - Any additional (variadic) arguments that we want to display on the screen.

   Outputs: (None)

   This function outputs a message (of a specific type/classification) to the screen.

   It's intended to communicate (important) information to the user, and it has different
   classification levels (which are defined in the messageType{} enum in Exceptions.h), which
   are shown to the user.

   For example, if you wanted to tell the user that something wrong had gone on, you could do:
   - Message(Fail, "Something wrong has happened.");

*/

void Message(messageType Type, char* String, ...) {

  // Prepare a va_list (the last non-variadic argument is String)

  va_list Arguments;
  va_start(Arguments, String);

  // Depending on the 'severity level', temporarily turn on the debug flag.

  bool SaveDebug = DebugFlag;

  switch (Type) {

    case Fail:
      [[fallthrough]];
    case Warning:
      [[fallthrough]];
    case Error:
      DebugFlag = true;

    default:
      break;

  }

  // Show the message type / 'classification level' to the user, enclosed by square
  // brackets (for example, [Boot], [Ok], etc).

  Print("[", 0x07);

  switch (Type) {

    case Info:
      Print("Info", 0x0F);
      break;

    case Boot:
      Print("Boot", 0x0B);
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

  Print("] ", 0x07);

  // Show the given message to the user, restore the value of Debug, and call va_end().

  vPrintf(String, ((Type != Info) ? 0x0F : 0x07), Arguments);
  Print("\n", 0x0F);

  DebugFlag = SaveDebug;
  va_end(Arguments);

}


/* [[noreturn]] void Panic()

   Inputs: char* String - The error/panic message to be displayed to the user.
           uint32 Eip - The instruction pointer of what caused the error (if not applicable,
                        use 0).

   Outputs: (None)

   This function essentially just shows an error message on the screen (and the address of the
   instruction/function that caused it), and then halts the system completely, forcing the
   user to restart.

   The purpose of this function is pretty much just to stop the boot process entirely if
   something goes wrong - for example, if there's an exception (like a division by 0), this
   function should be called.

*/

[[noreturn]] void Panic(char* String, uint32 Eip) {

  // Just in case, we want to disable interrupts, as they could interrupt the process

  __asm__("cli");

  // Enable the Debug flag (since this is an important system message), and show the error
  // message to the user.

  Putchar('\n', 0);

  DebugFlag = true;
  Message(Error, String);

  // Show the instruction pointer, if applicable

  if (Eip != 0) {
    Message(Info, "Instruction pointer (EIP) was %xh.", Eip);
  }

  // Finally, tell the user that the system will halt indefinitely (and that they should
  // probably restart their computer), and then do exactly that.

  Print("An error has occured, and this system will now halt indefinitely.\n", 0x07);
  Print("Please restart your computer.\n", 0x07);

  for(;;) {
    __asm__("cli; hlt");
  }

}
