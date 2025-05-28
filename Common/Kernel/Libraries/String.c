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
