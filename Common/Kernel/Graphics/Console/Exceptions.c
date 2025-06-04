// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
#include "Console.h"

/* void Message()

   Inputs: messageType{} Type - The message type/classification (`Kernel`,
           `Ok`, `Warning`, etc.);

           const char* String - The content of the message, as a string.

           (...) - Any additional (variadic) arguments that we want
           to display on the screen.

   Outputs: (None)

   This function outputs a message (of a specific type/classification) to the
   screen.

   It's intended to communicate (important) information to the user, and it
   has different classification levels (defined in the messageType{} enum
   in Graphics.h), which are shown to the user.

   Unlike other printing functions, this function doesn't necessarily need
   the Debug flag to be true for some message types, but `ConsoleEnabled`
   *must* be true.

   For example, if you wanted to tell the user that something had gone
   wrong, you could do:

   -> Message(Fail, "Something wrong has happened.");

*/

void Message(messageType Type, const char* String, ...) {

  // (If the console is disabled, return - we can't display anything.)

  if (ConsoleEnabled == false) {
    return;
  }

  // Prepare a va_list (the last non-variadic argument is String)

  va_list Arguments;
  va_start(Arguments, String);

  // Some messages are important; some aren't - depending on the severity
  // level of the message, set `Important` to either true or false.

  bool Important = false;

  switch (Type) {

    case Fail:
      [[fallthrough]];
    case Warning:
      [[fallthrough]];
    case Error:
      Important = true;

    default:
      break;

  }

  // Show the message type / 'classification level' to the user, enclosed
  // by square brackets (for example, [Boot], [Ok], etc).

  Print("[", Important, 0x07);

  switch (Type) {

    case Info:
      Print("Info", Important, 0x0F);
      break;

    case Kernel:
      Print("Kernel", Important, 0x03);
      break;

    case Ok:
      Print("Ok", Important, 0x0A);
      break;

    case Fail:
      Print("Fail", Important, 0x0C);
      break;

    case Warning:
      Print("Warning", Important, 0x0D);
      break;

    case Error:
      Print("Error", Important, 0x04);
      break;

    default:
      Print("Unknown", Important, 0x08);

  }

  Print("] ", Important, 0x07);

  // Show the given message to the user, and call va_end().

  vPrintf(String, Important, ((Type != Info) ? 0x0F : 0x07), Arguments);
  Print("\n\r", Important, ((Type != Info) ? 0x0F : 0x07));

  va_end(Arguments);

}
