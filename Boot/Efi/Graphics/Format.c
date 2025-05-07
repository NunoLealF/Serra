// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "Graphics.h"

/* int Strlen()

   Inputs: const char16* String - The string you want to get the length of.
   Outputs: int - The length of the given string.

   This function calculates the length of a (regular, 'wide' null-terminated)
   string by counting the amount of characters in it before getting to the
   final byte (which should be u'\0').

   It's assumed that the string in question is a regular EFI-compatible 'wide'
   UTF-16 string (*not* ASCII, UTF-8 or UTF-32), where each character is 16
   bits.

   As an example, if you wanted to know the length of u"abc", you could do:
   -> int AbcLength = Strlen(u"abc");

*/

int Strlen(const char16* String) {

  int Length = 0;

  // Go through each byte in the string, and only stop when we hit a null
  // character (u'\0').

  while (String[Length] != u'\0') {
    Length++;
  }

  // Return the string's length.

  return Length;

}


/* bool Strcmp()

   Inputs: const char16* StringA - The string we want to compare with StringB.
           const char16* StringB - The string we want to compare with StringA.

   Outputs: bool - Whether the two strings are equal.

   Unlike most other data types in C, strings can't be directly compared with
   ==, because they function as *pointers*.

   This means that, even if two strings are otherwise equal, comparing them
   directly *only compares their addresses, not their content*.

   The purpose of this function is to remedy that by comparing the *content*
   of both strings; for example, in:

   - const char* strA = u"abc";
   - const char* strB = u"abc";

   -> (strA == strB) returns *false*, because A and B are in different places;
   -> Strcmp(strA, strB) returns *true*, because A and B have the same content.

*/

bool Strcmp(const char16* StringA, const char16* StringB) {

  // Compare each character of each string, until we hit a null byte,
  // or until we find a different character.

  unsigned int Position = 0;

  while (StringA[Position] == StringB[Position]) {

    if (StringA[Position] == u'\0') {
      return true;
    }

    Position++;

  }

  return false;

}


/* bool StrcmpShort()

   Inputs: const char8* StringA - The string we want to compare with StringB.
           const char8* StringB - The string we want to compare with StringA.

   Outputs: bool - Whether the two strings are equal.

   (This function functions the same way as Strcmp(), but for short (char8*)
   strings instead of wide (char16*) ones.)

*/

bool StrcmpShort(const char8* StringA, const char8* StringB) {

  // Compare each character of each string, until we hit a null byte,
  // or until we find a different character.

  unsigned int Position = 0;

  while (StringA[Position] == StringB[Position]) {

    if (StringA[Position] == '\0') {
      return true;
    }

    Position++;

  }

  return false;

}


/* char16* Strrev()

   Inputs: char16* String - The string you want to reverse.
   Outputs: char16* - The reversed string. (Same string as the input one)

   This function serves one pretty simple purpose - it reverses a (wide,
   null-terminated) string. For example, if you wanted to reverse a string
   called ExampleEfiString, you could do the following:

   -> ExampleEfiString = Strrev(ExampleEfiString);

*/

char16* Strrev(char16* String) {

  // In order to reverse our string, we'll be using two indexes/pointers; one
  // at the very start of our string, and another at the very end.

  int Start = 0;
  int End = Strlen(String) - 1;

  // Now, all we need to do is to swap String[Start] with String[End], and
  // to increment or decrement those two variables until they both meet
  // the middle.

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


/* char16* Itoa()

   Inputs: uint64 Number - The (unsigned) integer we want to convert into a string.
           char16* Buffer - The buffer we want to use in our conversion.
           uint8 Base - The number base we want to use in our conversion.
                        Also known as the 'radix' number in some implementations.

   Outputs: char* - The (wide) output string, converted from the given integer.

   This function essentially just converts an (unsigned) integer into a (wide)
   string.

   It takes three parameters; the integer we want to convert into a string
   (Number), a buffer for the function to play around with (Buffer - *needs to
   be larger than the output*), and the number base/radix we want to use.

   This function accepts numbering systems between 2 and 36, which encompasses
   most numbering systems. Some common examples are:
   - Binary, which corresponds to base 2 (0-1);
   - Decimal, which corresponds to base 10 (0-9);
   - Hexadecimal, which corresponds to base 16 (0-F);

   For example, if you wanted to convert the hexadecimal number 7C00h into the
   string u"7C00h", you could do:
   -> char16 Buffer[100];
   -> char16* ExampleStringB = Itoa(0x7C00, Buffer, 16);

*/

char16* Itoa(uint64 Number, char16* Buffer, uint8 Base) {

  // It isn't really possible to convert using bases below 2 or 36, so if
  // that's the case, just return an empty string.

  if ((Base < 2) || (Base > 36)) {

    Buffer[0] = u'\0';
    return Buffer;

  }

  // If the given integer is just 0, then we'll want to return a string that
  // just contains "0". (The following code can't handle numbers below 1, so
  // this ends up being necessary)

  if (Number == 0) {

    Buffer[0] = u'0';
    Buffer[1] = u'\0';
    return Buffer;

  }

  // Now that we've made sure that the number in question *can* be converted,
  // it's time to actually do so.

  int Index = 0;
  const char16* Reference = u"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  while (Number != 0) {

    uint64 Remainder = Number % Base;
    Number /= Base;

    Buffer[Index] = Reference[Remainder];

    Index++;

  }

  // At this point, we've successfully converted every single digit.. in
  // *reverse order*. In order to not have the string come out backwards,
  // we'll have to reverse it, and return it as such.

  Buffer[Index] = u'\0';
  return Strrev(Buffer);

}


/* void vPrintf()

   Inputs: const char16* String - The string we want to display on the screen;
           there should be one format specifier (%?) for each variadic
           argument given later on.

           int32 Color - The color attribute we want to display with.

           va_list Arguments - Any additional (variadic) arguments that we want
           to display on the screen, if necessary, but specified as a va_list.

   Outputs: (None)

   This function is just the version of Printf() that uses a regular va_list;
   for more information, please check the documentation on Printf().

   Please keep in mind that, as with any other terminal/console-related
   function, this only does anything if DebugFlag and SupportsConout == true.

*/

void vPrintf(const char16* String, int32 Color, va_list Arguments) {

  // If SupportsConOut or Debug are false, then don't print anything; the
  // former means we can't use text mode at all, whereas the latter means
  // we don't want to display any unimportant messages.

  if ((DebugFlag == false) || (SupportsConOut == false)) {
    return;
  }

  // We might need to call Itoa (like if %d, %i or %x are called), so we
  // should prepare a buffer.

  char16 Buffer[64] = {0};

  // Now, let's actually go through the string:

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
          Putchar((const char16)(va_arg(Arguments, int)), Color); // (const) char16 is promoted to int.
          break;

        // Strings (%s).

        case 's':
          Print(va_arg(Arguments, const char16*), Color);
          break;

        // Unsigned binary integers (%b).

        case 'b':
          Print(Itoa(va_arg(Arguments, uint64), Buffer, 2), Color);
          break;

        // Unsigned decimal integers (%d, %i).

        case 'd':
        case 'i':
          Print(Itoa(va_arg(Arguments, uint64), Buffer, 10), Color);
          break;

        // Unsigned hexadecimal integers (%x).

        case 'x':
          Print(Itoa(va_arg(Arguments, uint64), Buffer, 16), Color);
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

      // Since each format specifier is two characters, we need to increment
      // Position twice.

      Position++;

    }

    // We can finally increment Position and pass onto the next part of the
    // string.

    Position++;

  }

}


/* void Printf()

   Inputs: const char16* String - The string we want to display on the screen;
           there should be one format specifier (%?) for each variadic
           argument given later on.

           int32 Color - The color attribute we want to display with.

           (...) - Any additional (variadic) arguments that we want to display
           on the screen, if necessary; *each argument should always
           correspond to a format specifier in String.*

   Outputs: (None)

   This function is similar to Print() in that it displays a string on the
   screen, but it accepts additional formatting / variadic arguments.

   Because of this, it's much more powerful than Print(), since this function
   allows you to effortlessly display other characters, strings, integers,
   etc. on the screen.

   Each additional argument *must* correspond to one of the following format
   specifiers:

   - %c (const char16): Displays a character.
   - %s (const char16*): Displays a string.
   - %d, %i (uint64): Displays an unsigned decimal integer.
   - %x (uint64): Displays an unsigned hexadecimal integer.
   - %% (none): Doesn't correspond to any additional argument, but just
   displays '%'.

   For example, if you wanted to print the length of a string named UwU,
   you could do:
   -> Printf("The length of the given string is %d.", 0x0F, Strlen(UwU));

   Please keep in mind that, as with any other terminal/console-related
   function, *this only does anything if Debug and SupportsConout == true.*

*/

void Printf(const char16* String, int32 Color, ...) {

  // If SupportsConOut or Debug are false, then don't print anything; the
  // former means we can't use text mode at all, whereas the latter means
  // we don't want to display any unimportant messages.

  if ((DebugFlag == false) || (SupportsConOut == false)) {
    return;
  }

  // Prepare va_list, va_start, etc.
  // Refer to this: https://blog.aaronballman.com/2012/06/how-variable-argument-lists-work-in-c/

  va_list Arguments; // Create va_list
  va_start(Arguments, Color); // Color is the last non-variadic argument

  // Call vPrintf() - this function essentially just serves as a wrapper for
  // that.

  vPrintf(String, Color, Arguments);

  // Since we're done, call va_end().

  va_end(Arguments);

}
