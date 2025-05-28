// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../../String.h"
#include "../Graphics.h"
#include "../Fonts/Fonts.h"
#include "Console.h"



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



// (TODO - Like Print but for a single character)

void Putchar_Graphical(const char Character, uint8 Attribute) {

  // Construct a temporary string with our character, and call
  // Print_Graphical with it.

  char String[] = {Character, '\0'};
  Print_Graphical(String, Attribute);

  // Now that we're done, return.

  return;

}



// (TODO - Add a simple Print function, that interfaces with DrawBitmapFont(),
// and breaks up the input stream into lines that can be written all at once)

void Print_Graphical(const char* String, uint8 Attribute) {

  // (Convert our 'standard' 8-bit (background + foreground) color
  // attributes into their respective RGB values, like this)

  uint32 BackgroundColor = ConvertAttributeToRgb(Attribute >> 4);
  uint32 ForegroundColor = ConvertAttributeToRgb(Attribute & 0x0F);

  // (Iterate through the string, and try to figure out the necessary
  // "displacement" - this lets us scroll enough in advance, and as
  // a bonus, we also calculate the length through `Index`)

  int Index = 0;

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

      PosY += (PosX / ConsoleData.LimitX);
      PosX %= ConsoleData.LimitX;

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

  // (If `PosY` >= `ConsoleData.LimitY`, then that means we need to
  // scroll before we can continue, so let's do that)

  if (PosY >= ConsoleData.LimitY) {

    // First, let's calculate the amount of lines we need to *make room
    // for*, as well as the amount of lines we need to *scroll*.
    // as well as the amount of lines we need to *clear*.

    auto ClearLines = min((PosY - ConsoleData.LimitY + 1), (ConsoleData.LimitY - 1));
    auto OffsetLines = (ConsoleData.LimitY - ClearLines);

    // Scroll every line back by `ClearLines`, using Memcpy().
    // (TODO - This would benefit from double buffering, it's slow)

    uintptr Framebuffer = (uintptr)GraphicsData.Framebuffer;
    auto Multiplier = (BitmapFontData.Height * GraphicsData.Pitch);

    Memcpy((void*)Framebuffer,
           (const void*)(Framebuffer + (ClearLines * Multiplier)),
           (uint64)(OffsetLines * Multiplier));

    // (Clear out the last few lines, using Memset())

    Memset((void*)(Framebuffer + (OffsetLines * Multiplier)), 0, (ClearLines * Multiplier));

    // (Update the real PosY accordingly)

    ConsoleData.PosY -= ClearLines;

  }

  // (Now, that we've scrolled, we can call DrawBitmapFont() to draw each
  // line as needed.)

  uint16 InitialPosition[2] = {ConsoleData.PosX, ConsoleData.PosY};

  int TempIndex = 0;
  char TempString[Index];

  for (auto Character = 0; Character <= Index; Character++) {

    // (TODO - I need to essentially go through String again until I need
    // to show a new line, etc. again.. I apologize if this doesnt make
    // sense, I'm really tired)

    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)
    // (TODO: Actually do this properly)

  }

  // (Return.)

  return;

}
