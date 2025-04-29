// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Efi/Efi.h"
#include "Graphics.h"

/* void ClearTerminal()

   Inputs: (None)
   Outputs: (None)

   This function essentially just clears the terminal, using the
   gST->ConOut->ClearScreen() function.

   (Please keep in mind that this function only functions if
   SupportsConOut is true)

*/

void ClearTerminal(void) {

  // If we're in EFI text mode, then use ClearScreen().

  if (SupportsConOut == true) {
    gST->ConOut->ClearScreen(gST->ConOut);
  }

  return;

}


/* void Putchar()

   Inputs: const char16 Character - The character we want to display on the
           screen;

           int32 Color - The attribute color we want to display with.

   Outputs: (None)

   This function displays a character on the screen. Unlike in legacy/BIOS
   mode, the EFI firmware takes care of *most* things for us, so all we have
   to do is call the right function.

   In this case, this function call is already implemented in the Print()
   function, so all we need to do is wrap around it (efiOutputString only
   supports strings, not individual characters).

   (Please keep in mind that, as with any other terminal/console-related
   function, this only does anything if Debug and SupportsConOut == true)

*/

void Putchar(const char16 Character, int32 Color) {

  // If SupportsConOut or Debug are false, then don't print anything; the
  // former means we can't use text mode at all, whereas the latter means
  // we don't want to display any unimportant messages.

  if ((Debug == false) || (SupportsConOut == false)) {
    return;
  }

  // Make a one-character string, and call Print; EFI doesn't allow us
  // to print individual characters, only strings, so this is necessary.

  const char16 Temp[] = {Character, u'\0'};
  Print(Temp, Color);

  // Now that we're done, we can return

  return;

}


/* void Print()

   Inputs: const char16* String - The string we want to display on the screen;
           int32 Color - The attribute color we want to display with.

   Outputs: (None)

   This function displays a (wide, UTF-16) string on the screen, serving as a
   wrapper around the gST->ConOut->OutputString() function.

   It essentially serves the same function as Putchar(), but for strings
   instead of individual characters.

   (Please keep in mind that, as with any other terminal/console-related
   function, this only does anything if Debug and SupportsConOut == true)

*/

void Print(const char16* String, int32 Color) {

  // If SupportsConOut or Debug are false, then don't print anything; the
  // former means we can't use text mode at all, whereas the latter means
  // we don't want to display any unimportant messages.

  if ((Debug == false) || (SupportsConOut == false)) {
    return;
  }

  // Save the current attribute, and then display the string.

  int32 SaveColor = gST->ConOut->Mode->Attribute;

  gST->ConOut->SetAttribute(gST->ConOut, Color);
  gST->ConOut->OutputString(gST->ConOut, (char16*)(String));

  gST->ConOut->SetAttribute(gST->ConOut, SaveColor);

  // Now that we're done, we can return

  return;

}
