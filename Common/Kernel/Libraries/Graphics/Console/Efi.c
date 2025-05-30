// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../../String.h"
#include "../../../Constructors/Firmware/Firmware.h"
#include "Console.h"

// (Converts a char*/char8* string to char16*)

static char16* ConvertToWideString(char16* Buffer, const char* String) {

  // Copy every character in `String` to its respective position in
  // `Buffer` (for every n, Buffer[n] = String[n]))

  for (auto Index = 0; Index <= Strlen(String); Index++) {
    Buffer[Index] = (char16)(String[Index]);
  }

  // Return `Buffer`.

  return Buffer;

}



// (TODO - Interfaces with gST->ConOut, prints a string)

void Print_Efi(const char* String, int32 Attribute) {

  // First, let's convert our ASCII string into a UTF-16 'wide'
  // string that can be understood by the firmware.

  char16 Buffer[Strlen(String)];
  char16* WideString = ConvertToWideString(Buffer, String);

  // Next, let's temporarily save the current color attribute,
  // before setting the one we were given (`Attribute`).

  int32 SaveAttribute = gST->ConOut->Mode->Attribute;
  gST->ConOut->SetAttribute(gST->ConOut, Attribute);

  // Finally, let's actually output the string we were given.

  // (EFI takes care of everything for us, including updating the
  // position and scrolling, so we don't have to do anything else)

  gST->ConOut->OutputString(gST->ConOut, WideString);

  // (Restore the previous color attribute, and return)

  gST->ConOut->SetAttribute(gST->ConOut, SaveAttribute);
  return;

}



// (TODO - Interfaces with gST->ConOut, prints a single character)

void Putchar_Efi(const char Character, int32 Attribute) {

  // Temporarily store the current color attribute, and set the
  // one we were given.

  int32 SaveAttribute = gST->ConOut->Mode->Attribute;
  gST->ConOut->SetAttribute(gST->ConOut, Attribute);

  // Create a 'wide' string (one character long) with our character,
  // and display it using ..->OutputString()

  char16 String[] = {(char16)Character, u'\0'};
  gST->ConOut->OutputString(gST->ConOut, String);

  // (Restore the previous color attribute, and return)

  gST->ConOut->SetAttribute(gST->ConOut, SaveAttribute);
  return;

}
