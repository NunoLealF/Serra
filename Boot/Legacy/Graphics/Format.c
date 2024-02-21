// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"
#include "Graphics.h"

/* int Strlen()

   Inputs: const char* String - The string you want to get the length of.
   Outputs: int - The length of the given string.

   This function calculates the length of a (regular, null-terminated) string by counting the
   amount of characters in it before getting to the final byte (which should be '\0').

   It's assumed that the string in question is a regular ASCII or UTF-8 string (not UTF-16 or
   UTF-32, where each character is 16 or 32 bits respectively).

   For example, if you wanted to know the length of a given string, you could do the following:
   - int Length = Strlen(ExampleString);

*/

int Strlen(const char* String) {

  int Length = 0;

  // Go through each byte in the string, and only stop when we hit a '\0' (null byte).

  while (String[Length] != '\0') {
    Length++;
  }

  // Return the string's length.

  return Length;

}


/* char* Strrev()

   Inputs: char* String - The string you want to reverse.
   Outputs: char* - The reversed string. (Same string as the input one, actually)

   This function serves one pretty simple purpose - it reverses a (null-terminated) string.

   Like every other function that involves strings, it's assumed that the string in question is
   ASCII or UTF-8, not UTF-16 or UTF-32 (which are 16 and 32 bytes respectively).

   For example, if you wanted to reverse the string ExampleString, you could do the following:
   - ExampleString = Strrev(ExampleString);

*/

char* Strrev(char* String) {

  // In order to reverse our string, we'll be using two indexes/pointers; one of them at the
  // very start of our string, and one of them at the very end (before the null byte).

  uint32 Start = 0;
  uint32 End = Strlen(String) - 1;

  // Now, all we need to do is to swap String[Start] with String[End], and to increment or
  // decrement those two variables until they both meet the middle.

  while (Start < End) {

    char Save = String[Start];

    String[Start] = String[End];
    String[End] = Save;

    Start++;
    End--;

  }

  // We can now return our reversed string!

  return String;

}


/* char* Itoa()

   Inputs: uint32 Number - The (unsigned) integer we want to convert into a string.
           char* Buffer - The buffer we want to use in our conversion.
           uint8 Base - The number base we want to use in our conversion.
                        Also known as the 'radix' number in some implementations.

   Outputs: char* - The output string, converted from the given integer.

   This function essentially just converts an (unsigned) integer into a string.

   It takes three parameters; the integer we want to convert into a string (Number), a buffer
   for the function to play around with (Buffer - needs to be larger than the output string),
   and the number base/radix we want to use (Base).

   Those last two parameters might be a little hard to understand, so, essentially - the Buffer
   is basically just the space we'll use to 'compose' our output string (which means it can't be
   smaller than the output), and the Base is simply just the numbering system we'll be using.

   This function accepts numbering systems between 2 and 36, which encompasses most numbering
   systems. Some common examples are:
   - Binary, which corresponds to base 2 (0-1);
   - Decimal, which corresponds to base 10 (0-9);
   - Hexadecimal, which corresponds to base 16 (0-F);

   For example, if you wanted to convert the hexadecimal number 7C00h into the string "7C00h",
   you could do:
   - char Buffer[100];
   - char* ExampleStringB = Itoa(0x7C00, Buffer, 16);

*/

char* Itoa(uint32 Number, char* Buffer, uint8 Base) {

  // It isn't really possible to convert using bases below 2 or 36, so if that's the case,
  // just return an empty string.

  if ((Base < 2) || (Base > 36)) {

    Buffer[0] = '\0';
    return Buffer;

  }

  // If the given integer is just 0, then we'll want to return a string that just contains "0".
  // The following code can't handle numbers below 1, so this ends up being necessary

  if (Number == 0) {

    Buffer[0] = '0';
    Buffer[1] = '\0';
    return Buffer;

  }

  // Now that we've made sure that the number in question *can* be converted, it's time to
  // actually do so.

  int Index = 0;
  const char* Reference = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  while (Number != 0) {

    uint32 Remainder = Number % Base;
    Number /= Base;

    Buffer[Index] = Reference[Remainder];

    Index++;

  }

  // At this point, we've successfully converted every single digit.. in reverse order.
  // In order to not have the string come out backwards, we'll reverse it, and return it as such.

  Buffer[Index] = '\0';
  return Strrev(Buffer);

}


/* void Printf()

   Inputs: const char* String - The string we want to display on the screen - there should be
           one format specifier (%?) for each variadic argument given later on.

           uint8 Color - The (VGA) color we want to display on the screen.

           (...) - Any additional (variadic) arguments that we want to display on the screen,
           if necessary. Each argument should always correspond to a format specifier in String.

   Outputs: (None)

   This function is similar to Print() in that it displays a string on the screen, but it
   accepts additional formatting / variadic arguments.

   Because of this, it's much more powerful than Print(), since this function allows you to
   effortlessly display other characters, strings, integers, etc. on the screen. Each additional
   argument must correspond to one of the following format specifiers:

   - %c (const char): Displays a character.
   - %s (const char*): Displays a string.
   - %d, %i (unsigned int): Displays an unsigned decimal integer.
   - %x (unsigned int): Displays an unsigned hexadecimal integer.
   - %% (none): Doesn't correspond to any additional argument, but just displays '%'.

   For example, if you wanted to print the length of a given string named UwU, you could do:
   - Printf("The length of the given string is %d.", 0x0F, Strlen(UwU));

*/

void Printf(const char* String, uint8 Color, ...) {

  // Prepare va_list, va_start, etc.
  // Refer to this: https://blog.aaronballman.com/2012/06/how-variable-argument-lists-work-in-c/

  va_list Arguments; // Create va_list
  va_start(Arguments, Color); // Color is the last non-variadic argument

  // We might need to call Itoa (like if %d, %i or %x are called), so we should prepare
  // a buffer.

  char Buffer[16];
  Memset(Buffer, '\0', 16);

  // Now, we can actually go through the string!

  int Position = 0;

  while (Position < Strlen(String)) {

    if (String[Position] != '%') {

      // Print the character as normal.

      Putchar(String[Position], Color);

    } else {

      // Interpret the format specifier after our %.

      switch (String[Position+1]) {

        // Characters (%c).

        case 'c':
          Putchar((const char)va_arg(Arguments, int), Color); // (const) char is promoted to int.
          break;

        // Strings (%s).

        case 's':
          Print(va_arg(Arguments, const char*), Color);
          break;

        // Unsigned decimal integers (%d, %i).

        case 'd':
        case 'i':
          Print(Itoa(va_arg(Arguments, unsigned int), Buffer, 10), Color);
          break;

        // Unsigned hexadecimal integers (%x).

        case 'x':
          Print(Itoa(va_arg(Arguments, unsigned int), Buffer, 16), Color);
          break;

        // Special cases (end of string, percent sign, unknown, etc.).

        case '\0':
        case '%':
          Putchar('%', Color); // Only circumstance where you don't use va_list
          break;

        default:
          (void)va_arg(Arguments, int); // Just increment va_arg and then ignore it
          break;

      }

      // Since each format specifier is two characters, we need to increment Position twice.

      Position++;

    }

    // We can finally increment Position and pass onto the next part of the string.

    Position++;

  }

  // Since we're done, call va_end().

  va_end(Arguments);

}
