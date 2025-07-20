// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../../Libraries/Stdint.h"
#include "../../../Libraries/String.h"
#include "../../../Firmware/Firmware.h"
#include "../../Graphics.h"
#include "../../Fonts/Fonts.h"
#include "../Console.h"
#include "Graphical.h"

// (TODO - This function converts a color attribute into actual RGB
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



// (TODO - This function scrolls using the best possible method, updating
// the backbuffer as well in the process)

static void Scroll_Graphical(uintptr Lines) {

  // First, let's calculate the number of lines we need to clear.

  auto ClearLines = (ConsoleData.LimitY - Lines);

  // Next, let's update the back buffer. We need to scroll everything
  // back by `Lines`, and clear the last `ClearLines`.

  // (TODO - The `Memmove` here is by far the slowest part, maybe optimize this?)
  // (TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO)

  auto Multiplier = (BitmapFontData.Height * GraphicsData.Pitch);

  Memmove((void*)GraphicsData.Buffer,
          (const void*)((uintptr)GraphicsData.Buffer + (ClearLines * Multiplier)),
          (uintptr)(Lines * Multiplier));

  Memset((void*)((uintptr)GraphicsData.Buffer + (Lines * Multiplier)),
         (uint8)0, (ClearLines * Multiplier));

  // (Update the current vertical position)

  ConsoleData.PosY -= ClearLines;

  // Now, depending on the graphics type (and the presence of hardware
  // acceleration, like GOP's BLT), we'll either:

  // (TODO - I don't have a proper way to rapidly copy things to the
  // framebuffer, usually this wouldn't be necessary and you'd just
  // have some sort of FramebufferMemcpy() function)

  bool SameAsGopBltFormat = (TranslateRgbColorValue(0xABCDEF) == 0xABCDEF);

  if ((GraphicsData.Gop == NULL) || (SameAsGopBltFormat == false)) {

    // (A) Copy directly from the backbuffer to the framebuffer

    Memcpy((void*)GraphicsData.Framebuffer,
           (const void*)GraphicsData.Buffer,
           (uintptr)(GraphicsData.LimitY * GraphicsData.Pitch));

  } else {

    // (B) Use GOP functions to quickly move data to the framebuffer.

    efiGraphicsOutputProtocol* Gop = GraphicsData.Gop;

    if (GraphicsData.Pitch == (GraphicsData.Bpp * GraphicsData.LimitX)) {

      // (Copy everything in one go)

      Gop->Blt(Gop, (volatile efiGraphicsOutputBltPixel*)GraphicsData.Buffer,
               EfiBltBufferToVideo, 0, 0, 0, 0,
               GraphicsData.LimitX, GraphicsData.LimitY, 0);

    } else {

      // (Individually copy each line)

      for (uint16 Line = 0; Line < GraphicsData.LimitY; Line++) {

        Gop->Blt(Gop, (volatile efiGraphicsOutputBltPixel*)GraphicsData.Buffer,
                 EfiBltBufferToVideo, 0, Line, 0, Line,
                 GraphicsData.LimitX, 1, 0);

      }

    }

  }

  // Now that we're done, let's return.

  return;

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

    auto Lines = min((PosY + 1 - ConsoleData.LimitY), ConsoleData.LimitY);
    Scroll_Graphical(ConsoleData.LimitY - Lines);

  }

  // Finally, now that we've finished scrolling, we can call DrawBitmapFont()
  // to draw each line as needed.

  uintptr BufferPosition[2] = {(ConsoleData.PosX * BitmapFontData.Width),
                               (ConsoleData.PosY * BitmapFontData.Height)};

  uintptr InitialPosition[2] = {BufferPosition[0], BufferPosition[1]};

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
      DrawBitmapFont(LineString, &BitmapFontData, false, ForegroundColor, BackgroundColor, BufferPosition[0], BufferPosition[1]);

      // (Reset LineIndex, to start a new line.)

      LineIndex = 0;

      // (Update BufferPosition[] to correspond to the start of the next
      // line's string, if applicable.)

      BufferPosition[0] = (ConsoleData.PosX * BitmapFontData.Width);
      BufferPosition[1] = (ConsoleData.PosY * BitmapFontData.Height);

    }

  }

  // Now that we've drawn everything, we can copy over everything to the
  // framebuffer.

  // (Calculate offset)

  const uintptr Start = ((InitialPosition[0] * GraphicsData.Bpp) +
                         (InitialPosition[1] * GraphicsData.Pitch));

  const uintptr End = ((BufferPosition[0] * GraphicsData.Bpp) +
                       (BufferPosition[1] * GraphicsData.Pitch));

  // (Get back+front buffers)

  const uintptr Backbuffer = ((uintptr)GraphicsData.Buffer + Start);
  const uintptr Framebuffer = ((uintptr)GraphicsData.Framebuffer + Start);

  // (Actually copy the data itself.)

  Memcpy((void*)Framebuffer, (const void*)Backbuffer, (End - Start + 1));

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
