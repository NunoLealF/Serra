// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
#include "Console.h"

// (TODO - Add a simple Putchar/Print function, as in the BIOS loader)

void Print_Vga(const char* String, uint8 Attribute) {

  // Iterate through the string, and try to figure out the necessary
  // "displacement" - this lets us scroll enough in advance (and as
  // a bonus, we also calculate the length through `Index`).

  auto Index = 0;

  uint32 PosX = ConsoleData.PosX;
  uint32 PosY = ConsoleData.PosY;

  uint32 Threshold = (PosY + 1);

  while (String[Index] != '\0') {

    // (Depending on the character type..)

    switch (String[Index]) {

      case '\n':
        PosY++;
        [[fallthrough]];
      case '\r':
        PosX = 0;
        break;

      default:
        PosX++;

    }

    // (Check if we've crossed over to a new line, and if we have,
    // update the necessary variables)

    if (PosX > ConsoleData.LimitX) {

      PosX -= (ConsoleData.LimitX + 1);
      PosY++;

    }

    if (PosY >= Threshold) {
      Threshold = (PosY + 1);
    }

    // (If we've exceeded the maximum height, then *stop* - we won't
    // display any more of this for now)

    if (Threshold >= (ConsoleData.LimitY * 2)) {
      break;
    }

    // (Increment Index.)

    Index++;

  }

  // Now that we know the displacement of our string, let's check whether
  // we need to scroll (PosY >= ConsoleData.LimitY), and if so, do that.

  if (PosY >= ConsoleData.LimitY) {

    // (Calculate the amount of lines we need to make room for, and the
    // amount of lines we need to scroll (`ClearLines` and `ScrollLines`))

    auto ClearLines = min((PosY + 1 - ConsoleData.LimitY), ConsoleData.LimitY);
    auto ScrollLines = (ConsoleData.LimitY - ClearLines);

    // (Scroll every line back by `ClearLines`, using Memcpy() - keep in
    // mind that each VGA character is 2 bytes long)

    auto Multiplier = (ConsoleData.LimitX * 2);

    Memcpy((void*)((uintptr)0xB8000),
           (const void*)((uintptr)0xB8000 + (ClearLines * Multiplier)),
           (uint64)(ScrollLines * Multiplier));

    // (Clear out the last few lines, using Memset(), and update PosY
    // to correspond to the start of those lines)

    Memset((void*)((uintptr)0xB8000 + (ScrollLines * Multiplier)),
           (uint8)0x00,
           (ClearLines * Multiplier));

    ConsoleData.PosY -= ClearLines;

  }

  // Finally, now that we've finished scrolling, we can draw each
  // character on the framebuffer as normal.

  uintptr Framebuffer = 0xB8000 + ((ConsoleData.PosX + (ConsoleData.PosY * ConsoleData.LimitX)) * 2);

  for (auto Position = 0; Position < Index; Position++) {

    bool IsSpecialCharacter = false;

    switch (String[Position]) {

      // (Handle special characters - these need `IsSpecialCharacter`
      // to be set to true)

      case '\n':
        ConsoleData.PosY++;
        [[fallthrough]];
      case '\r':
        ConsoleData.PosX = 0;
        IsSpecialCharacter = true;
        break;

      // (Handle regular characters)

      default:
        ConsoleData.PosX++;

    }

    // (If we've exceeded a line boundary, move onto the next line)

    if (ConsoleData.PosX >= ConsoleData.LimitX) {

      ConsoleData.PosX -= ConsoleData.LimitX;
      ConsoleData.PosY++;

    }

    // (If we *aren't* dealing with a special character, draw the
    // current character)

    if (IsSpecialCharacter == false) {

      // (Draw a VGA character to `Framebuffer` - in this case, the
      // highest/lowest bytes represent the color/character)

      uint16* Buffer = (uint16*)Framebuffer;
      uint16 Character = ((Attribute << 8) | String[Position]);

      *Buffer = Character;

      // (Move `Framebuffer` to point to the next character)

      Framebuffer += 2;

    }

  }

  // (Return.)

  return;

}



// (TODO - Putchar)

void Putchar_Vga(const char Character, uint8 Attribute) {

  // Construct a temporary string with our character, and call
  // Print_Graphical with it.

  char String[] = {Character, '\0'};
  Print_Vga(String, Attribute);

  // Now that we're done, return.

  return;

}
