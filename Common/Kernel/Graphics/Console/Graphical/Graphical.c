// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Libraries/Stdint.h"
#include "../../../Libraries/String.h"
#include "../../Graphics.h"
#include "../../Fonts/Fonts.h"
#include "../Console.h"
#include "Graphical.h"

// (TODO - This macro converts a color attribute into actual RGB
// color values, which is nice)

static inline uint32 ConvertAttributeToRgb(uint8 Attribute) [[reproducible]] {

  // The 'standard' 4-bit color attribute (used by CGA/VGA and EFI) is
  // actually relatively easy to understand:

  // -> Bit 0 corresponds to blue (0000AAh);
  // -> Bit 1 corresponds to green (00AA00h);
  // -> Bit 2 corresponds to red (AA0000h);
  // -> Bit 3 corresponds to intensity (add 555555h).

  uint32 Color = 0;

  if ((Attribute & (1ULL << 0)) != 0) Color += 0xAA; // (Blue bit)
  if ((Attribute & (1ULL << 1)) != 0) Color += 0xAA00; // (Green bit)
  if ((Attribute & (1ULL << 2)) != 0) Color += 0xAA0000; // (Red bit)
  if ((Attribute & (1ULL << 3)) != 0) Color += 0x555555; // (Intensity)

  // (Return our converted RGB color.)

  return Color;

}



// (TODO - Add a simple Print function, that interfaces with DrawBitmapFont(),
// and breaks up the input stream into lines that can be written all at once)

void Print_Graphical(const char* String, uint8 Attribute) {

  // (Convert our 'standard' 8-bit (background + foreground) color
  // attributes into their respective RGB values, like this)

  uint32 BackgroundColor = ConvertAttributeToRgb(Attribute >> 4);
  uint32 ForegroundColor = ConvertAttributeToRgb(Attribute & 0x0F);

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

    // First, let's calculate the amount of lines we need to *make room
    // for*, as well as the amount of lines we need to *scroll*.
    // as well as the amount of lines we need to *clear*.

    auto ClearLines = min((PosY + 1 - ConsoleData.LimitY), ConsoleData.LimitY);
    auto ScrollLines = (ConsoleData.LimitY - ClearLines);

    // (Scroll every line back by `ClearLines`, using Memcpy())

    uintptr Buffer = (uintptr)GraphicsData.Buffer;
    auto Multiplier = (BitmapFontData.Height * GraphicsData.Pitch);

    Memcpy((void*)Buffer,
           (const void*)(Buffer + (ClearLines * Multiplier)),
           (uint64)(ScrollLines * Multiplier));

    // (Clear out the last few lines, using Memset(), and update PosY
    // to correspond to the start of those lines)

    Memset((void*)(Buffer + (ScrollLines * Multiplier)),
           (uint8)0x00,
           (uint64)(ClearLines * Multiplier));

    ConsoleData.PosY -= ClearLines;

    // (Copy the entire back buffer to the framebuffer in one operation;
    // that way, we only copy to the framebuffer once.)

    Memcpy((void*)GraphicsData.Framebuffer,
           (const void*)GraphicsData.Buffer,
           (uint64)(GraphicsData.LimitY * GraphicsData.Pitch));

  }

  // Finally, now that we've finished scrolling, we can call DrawBitmapFont()
  // to draw each line as needed.

  uint16 InitialPosition[2] = {ConsoleData.PosX, ConsoleData.PosY};

  auto LineIndex = 0;
  char LineString[Index];

  for (auto Position = 0; Position <= Index; Position++) {

    bool DrawLine = false;

    switch (String[Position]) {

      // (Handle special characters - these need `DrawLine` to be
      // set to true)

      case '\n':
        ConsoleData.PosY++;
        [[fallthrough]];
      case '\r':
        ConsoleData.PosX = 0;
        [[fallthrough]];
      case '\0':
        DrawLine = true;
        break;

      // (Handle regular characters)

      default:
        ConsoleData.PosX++;
        LineString[LineIndex++] = String[Position];

    }

    // (If we've exceeded the current line's boundaries, move to the
    // next line; otherwise, commit the current character to LineString.)

    if (DrawLine == false) {

      if (ConsoleData.PosX >= ConsoleData.LimitX) {

        ConsoleData.PosX -= ConsoleData.LimitX;
        ConsoleData.PosY++;

        DrawLine = true;

      }

    }

    // (If `DrawLine` is true, draw the line we just finished, and move
    // onto the next line (if applicable))

    if (DrawLine == true) {

      // (Commit the string (corresponding to the line we just finished)
      // to the framebuffer.)

      LineString[LineIndex] = '\0';
      DrawBitmapFont(LineString, &BitmapFontData, ForegroundColor, BackgroundColor, (InitialPosition[0] * BitmapFontData.Width), (InitialPosition[1] * BitmapFontData.Height));

      // (Reset LineIndex, to start a new line.)

      LineIndex = 0;

      // (Update InitialPosition[] to correspond to the start of the next
      // line's string, if applicable.)

      InitialPosition[0] = ConsoleData.PosX;
      InitialPosition[1] = ConsoleData.PosY;

    }

  }

  return;

}



// (TODO - Like Print but for a single character)

void Putchar_Graphical(const char Character, uint8 Attribute) {

  // Construct a temporary string with our character, and call
  // Print_Graphical with it.

  const char String[] = {Character, '\0'};
  Print_Graphical(String, Attribute);

  // Now that we're done, return.

  return;

}
