// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
#include "Console.h"

/* void vPrintf(), static inline int vPrintf_FlushQueue()

   Inputs: const char* String - The string we want to display on the screen;
           there should be one format specifier (%?) for each variadic
           argument given later on.

           bool Important - Whether the message should be displayed,
           even if the Debug flag is disabled.

           uint8 Color - The color we want to display on the screen.

           va_list Arguments - Any additional (variadic) arguments that we want
           to display on the screen, if necessary, but specified as a va_list.

   Outputs: (None)

   This function is just the version of Printf() that uses a regular va_list;
   for more information, please check the documentation on Printf().

   (This implementation has a helper function, vPrintf_FlushQueue(), which
   isn't really meant to be called outside of vPrintf())

*/

static inline int vPrintf_FlushQueue(char* Queue, int QueueLength, bool Important, uint8 Color) {

  Queue[QueueLength] = '\0';
  Print(Queue, Important, Color);
  QueueLength = 0;

  return QueueLength;

}

void vPrintf(const char* String, bool Important, uint8 Color, va_list Arguments) {

  // (If the console is disabled, or both the Debug flag and `Important`
  // is false, return)

  if (ConsoleEnabled == false) {
    return;
  } else if ((DebugFlag == false) && (Important == false)) {
    return;
  }

  // Putchar() and Print() both have a lot of overhead, so let's try to
  // minimize that by only printing when needed, by using a 'queue'.

  auto Length = Strlen(String);

  char Queue[Length];
  auto QueueLength = 0;

  // Additionally, we might need to call Itoa (like if %d, %i or %x are
  // used), so we should prepare a buffer.

  char Buffer[72] = {0};

  // Now, let's actually go through the string:

  auto Position = 0;

  while (Position <= Length) {

    if (String[Position] != '%') {

      // Are we dealing with a regular character, or with the null
      // character at the end of the string?

      if (String[Position] != '\0') {
        Queue[QueueLength++] = String[Position];
      } else {
        QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
      }

    } else {

      // Interpret the format specifier after our %.

      switch (String[Position+1]) {

        // Characters (%c).

        case 'c':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Putchar((const char)va_arg(Arguments, int), Important, Color); // (const) char is promoted to int.
          break;

        // Strings (%s).

        case 's':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Print(va_arg(Arguments, const char*), Important, Color);
          break;

        // Unsigned binary integers (%b).

        case 'b':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Print(Itoa(va_arg(Arguments, unsigned long long), Buffer, 2), Important, Color);
          break;

        // Unsigned decimal integers (%d or %i).

        case 'd':
          [[fallthrough]];
        case 'i':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Print(Itoa(va_arg(Arguments, unsigned long long), Buffer, 10), Important, Color);
          break;

        // Unsigned hexadecimal integers (%x).

        case 'x':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Print(Itoa(va_arg(Arguments, unsigned long long), Buffer, 16), Important, Color);
          break;

        // Special cases (end of string, percent sign, unknown, etc.).

        case '\0':
          [[fallthrough]];
        case '%':
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          Putchar('%', Important, Color); // Only circumstance where you don't use va_list
          break;

        default:
          QueueLength = vPrintf_FlushQueue(Queue, QueueLength, Important, Color);
          (void)va_arg(Arguments, int); // Just increment va_arg and then ignore it
          break;

      }

      // Since each format specifier is two characters, we need to increment
      // Position twice.

      Position++;

    }

    // We can finally increment Position and move onto the next part of
    // the string.

    Position++;

  }

}



/* void Printf()

   Inputs: const char* String - The string we want to display on the screen;
           there should be one format specifier (%?) for each variadic
           argument given later on.

           bool Important - Whether the message should be displayed,
           even if the Debug flag is disabled.

           uint8 Color - The color we want to display on the screen.

           (...) - Any additional (variadic) arguments that we want to
           display on the screen, if necessary. Each argument should
           always correspond to a format specifier in `String`.

   Outputs: (None)

   This function is similar to Print() in that it displays a string on the
   screen, but it accepts additional formatting / variadic arguments.

   Because of this, it's much more powerful than Print(), because it allows
   you to effortlessly display other characters, strings, integers, etc. on
   the screen.

   Each additional (variadic) argument must correspond to one of the
   following format specifiers:

   - %c (const char): Displays a character;
   - %s (const char*): Displays a string;
   - %d, %i (unsigned long long): Displays an unsigned decimal integer;
   - %x (unsigned long long): Displays an unsigned hexadecimal integer;
   - %% (none): Doesn't correspond to any additional argument, just displays '%'.

   For example, if you wanted to print the memory address of this very
   function, you could do:

   -> Printf("The memory address of Printf() is at %xh", true, 0x0F, &Printf);

*/

void Printf(const char* String, bool Important, uint8 Color, ...) {

  // (If the console is disabled, or both the Debug flag and `Important`
  // is false, return)

  if (ConsoleEnabled == false) {
    return;
  } else if ((DebugFlag == false) && (Important == false)) {
    return;
  }

  // (Prepare va_list, va_start, etc.)
  // Refer to this: https://blog.aaronballman.com/2012/06/how-variable-argument-lists-work-in-c/

  va_list Arguments; // Create va_list
  va_start(Arguments, Color); // Color is the last non-variadic argument

  // (Call vPrintf() - this function essentially just serves as a
  // wrapper around that)

  vPrintf(String, Important, Color, Arguments);

  // (Since we're done, call va_end())

  va_end(Arguments);

}
