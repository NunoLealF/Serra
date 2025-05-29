// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "String.h"

/* int Strlen()

   Inputs: const char* String - The string you want to measure.
   Outputs: int - The length of the given string.

   This function calculates the length of a ('regular', null-terminated)
   string, by counting the amount of characters in it, before the final
   byte, which should be '\0'.

   It's assumed that the string in question is a regular ASCII string
   (*not* UTF-16 or UTF-32), where each character is 8 bits wide.

   For example, if you wanted to know the length of "Serra", you
   could do:

   -> auto LengthOfSerra = Strlen("Serra");

*/

int Strlen(const char* String) {

  int Length = 0;

  // Go through each byte in the string, and only stop when we hit
  // a null character ('\0', or 00h).

  while (String[Length] != '\0') {
    Length++;
  }

  // Return the string's length.

  return Length;

}



/* int StrlenWide()

   Inputs: const char16* String - The 'wide' string you want to measure.
   Outputs: int - The length of the given string.

   This function calculates the length of a ('wide', null-terminated)
   string, by counting the amount of characters in it, before the
   final byte, which should be u'\0'.

   It's assumed that the string in question is a regular UTF-16 string
   (*not* ASCII, UTF-8 or UTF-32), where each character is 16 bits wide.

   For example, if you wanted to know the length of u"Serra", you
   could do:

   -> auto LengthOfSerra = StrlenWide(u"Serra");

*/

int StrlenWide(const char16* String) {

  int Length = 0;

  // Go through each byte in the string, and only stop when we hit
  // a null character (u'\0', or 0000h).

  while (String[Length] != u'\0') {
    Length++;
  }

  // Return the string's length.

  return Length;

}



/* char* Strrev()

   Inputs: char* String - The string you want to reverse.

   Outputs: char* - The reversed string. (Same string as the input one,
            actually)

   This function serves one pretty simple purpose - it reverses a (null-
   -terminated) string.

   Like every other function that involves strings, it's assumed that the
   string itself is 'short' (char* or char8*, ASCII), not 'wide' (UTF-16 or
   UTF-32).

   As an example, if you wanted to reverse the string "123", you could do:

   -> char* Example = "123";
   -> Example = Strrev(Example); // `Example` is now "321"

*/

char* Strrev(char* String) {

  // In order to reverse our string, we'll be using two indexes; one of them
  // at the start, and another one at the very end (right before '\0').

  int Start = 0;
  int End = Strlen(String) - 1;

  // Now, all we need to do is swap String[Start] with String[End], and to
  // increment/decrement those two indexes until they both meet each other.

  while (Start < End) {

    // (Swap String[Start] and String[End])

    char Save = String[Start];

    String[Start] = String[End];
    String[End] = Save;

    // (Update `Start` and `End` respectively)

    Start++;
    End--;

  }

  // We've now successfully reversed our string!

  return String;

}



/* char* Itoa()

   Inputs: uint64 Number - The unsigned integer we want to convert to a string.
           char* Buffer - The buffer we want to use in our conversion.
           uint8 Base - The number base we want to use in our conversion; also
                        known as the 'radix' number in some implementations.

   Outputs: char* - The output string, converted from the given integer.

   This function essentially just converts an integer into a string.

   It takes three parameters: the number we want to convert (`Number`), the
   base (or radix) we want to convert it in (`Base`), and a pointer to a
   buffer large enough for the output string (`Buffer`).

   This function accepts numbering systems between 2 and 36, which
   encompasses most situations. Some common systems are:

   - Binary, which corresponds to base 2 (0-1);
   - Decimal, which corresponds to base 10 (0-9);
   - Hexadecimal, which corresponds to base 16 (0-F);

   As an example, if you wanted to convert the (decimal, base 10) number
   `12345678` into the string "12345678", you could try to do:

   -> char Buffer[10];
   -> char* String = Itoa(-12345678, Buffer, 10);

*/

char* Itoa(uint64 Number, char* Buffer, uint8 Base) {

  // It isn't really possible to convert using bases below 2 or 36, so if
  // that's the case, just return an empty string.

  if ((Base < 2) || (Base > 36)) {

    Buffer[0] = '\0';
    return Buffer;

  }

  // The following code can only handle non-zero numbers, so if the number
  // we're looking to convert *is* zero, then return "0".

  if (Number == 0) {

    Buffer[0] = '0';
    Buffer[1] = '\0';
    return Buffer;

  }

  // Now that we've made sure that the number in question *can* be
  // converted, let's actually do that.

  int Index = 0;
  const char* Reference = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  while (Number != 0) {

    uint32 Remainder = Number % Base;
    Number /= Base;

    Buffer[Index++] = Reference[Remainder];

  }

  // At this point, we've successfully converted every single digit.. *in
  // reverse order*. In order to not have the string come out backwards,
  // we'll reverse it, and return it as such.

  Buffer[Index] = '\0';
  return Strrev(Buffer);

}
