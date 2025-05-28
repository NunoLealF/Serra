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

  // (TODO - My previous implementation of this was so borked that I'm
  // just trying it again entirely, f*** it (and f*** this sh**)

  // (Return.)

  return;

}
