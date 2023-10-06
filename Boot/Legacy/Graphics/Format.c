// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"

/* uint32 Strlen()

   Inputs:  const char* String - The string you want to get the length of.

   Outputs: uint32 - The length of the given string.

   This function calculates the length of a (regular, null-terminated) string by counting the
   amount of characters in it before getting to the final byte (which should be '\0').

   It's assumed that the string in question is a regular ASCII or UTF-8 string (not UTF-16 or
   UTF-32, where each character is 16 or 32 bits respectively).

   For example, if you wanted to know the length of a given string, you could do the following:
   > unsigned int Length = Strlen(ExampleString);

*/

uint32 Strlen(const char* String) {

  uint32 Length = 0;

  // Go through each byte in the string, and only stop when we hit a '\0' (null byte).

  while (String[Length] != '\0') {
    Length++;
  }

  // Return the string's length.

  return Length;

}


// String reverse

char* Strrev(char* String) {

  // Indexes

  uint32 Start = 0;
  uint32 End = Strlen(String) - 1; // We do -1 to avoid messing with the null byte

  // Reverse characters in the string.

  while (Start < End) {

    char Save = String[Start];

    String[Start] = String[End];
    String[End] = Save;

    Start++;
    End--;

  }

  return String;

}


// Itoa - unsigned only

char* Itoa(uint32 Number, char* Buffer, uint8 Base) {

  // If the base is below 2 (binary) or above 36 (0-9 + A-Z), return nothing.

  if ((Base < 2) || (Base > 36)) {

    Buffer[0] == '\0';
    return Buffer;

  }

  // Now that we've gotten that out of the way, let's actually handle this
  // First, let's try to handle 0.

  if (Number == 0) {

    Buffer[0] == '0';
    Buffer[1] == '\0';
    return Buffer;

  }

  // Do the thing

  int Index = 0;
  const char* Reference = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  while (Number != 0) {

    uint32 Remainder = Number % Base;
    Number /= Base;

    Buffer[Index] = Reference[Remainder];

    Index++;

  }

  // Reverse string, from 0 to (Index-1), and add a null byte at [Index]

  Buffer[Index] = '\0';
  return Strrev(Buffer);

}
