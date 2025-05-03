// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Graphics.h"

/* void Message()

   Inputs: messageType{} Type - The message type/classification (Boot, Ok, Warning, etc.);
           char16* String - The content of the message, as a single string.
           (...) - Any additional (variadic) arguments that we want to display on the screen.

   Outputs: (None)

   This function outputs a message (of a specific type/classification) to the
   screen.

   It's intended to communicate (important) information to the user, and it
   has different classification levels (defined in the messageType{} enum
   in Graphics.h), which are shown to the user.

   Unlike other printing functions, this function doesn't necessarily need
   DebugFlag to be true for some message types, but SupportsConOut *must*
   be true.

   For example, if you wanted to tell the user that something had gone
   wrong, you could do:
   -> Message(Fail, "Something wrong has happened.");

*/

void Message(messageType Type, char16* String, ...) {

  // If SupportsConOut is false, then we can't print anything at all,
  // so let's check for that first.

  if (SupportsConOut == false) {
    return;
  }

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

  // Show the message type / 'classification level' to the user, enclosed
  // by square brackets (for example, [Boot], [Ok], etc).

  Print(u"[", 0x07);

  switch (Type) {

    case Info:
      Print(u"Info", 0x0F);
      break;

    case Boot:
      Print(u"Boot", 0x0B);
      break;

    case Ok:
      Print(u"Ok", 0x0A);
      break;

    case Fail:
      Print(u"Fail", 0x0C);
      break;

    case Warning:
      Print(u"Warning", 0x0D);
      break;

    case Error:
      Print(u"Error", 0x04);
      break;

    default:
      Print(u"Unknown", 0x08);

  }

  Print(u"] ", 0x07);

  // Show the given message to the user, restore the value of Debug, and call
  // va_end().

  vPrintf(String, ((Type != Info) ? 0x0F : 0x07), Arguments);
  Print(u"\n\r", 0x0F);

  DebugFlag = SaveDebug;
  va_end(Arguments);

}
