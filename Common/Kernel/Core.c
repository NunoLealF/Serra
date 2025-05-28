// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if (GraphicsData.IsSupported == true) {

    for (auto Y = 0; Y < GraphicsData.LimitY; Y++) {

      // (Get necessary addresses and values)

      auto Buffer = ((uintptr)GraphicsData.Framebuffer + (GraphicsData.Pitch * Y));
      auto Size = (GraphicsData.LimitX * GraphicsData.Bpp);

      // (Use memset to display a pretty gradient)

      Memset((void*)Buffer, (uint8)(Y * 256 / GraphicsData.LimitY), Size);

    }

    for (uint32 Y2 = 0; Y2 < (GraphicsData.LimitY - BitmapFontData.Height) / 32; Y2++) {
      DrawBitmapFont("Hi, this is graphics-mode Serra! <3", &BitmapFontData, 0xFFFFFF, 0, true, 0, Y2*32);
      DrawBitmapFont("May 27 2025", &BitmapFontData, 0xFFFFFF, 0x41BCCB, false, 0, (Y2*32)+16);
    }

    // (Draw a window in the middle)

    uint16 WindowSize[2] = {400, 32};
    uint16 StartWinSz[2] = {((GraphicsData.LimitX - WindowSize[0]) / 2), ((GraphicsData.LimitY - WindowSize[1]) / 2)};

    DrawRectangle(0, StartWinSz[0], StartWinSz[1], WindowSize[0], WindowSize[1]);

    // (Display font characters)
    // (WARNING: Fonts must not exceed 8px width with initial driver (!))
    // (WARNING: This is kind of a mess)

    uintptr GlyphData = (uintptr)BitmapFontData.GlyphData;

    const char* TestString = "Hi, this is graphics-mode Serra! <3 May 27 2025"; // [48]

    for (uint16 Thingy = 0; Thingy < BitmapFontData.Height; Thingy++) {

      #define Tslen 47
      #define Cutoff 36
      uint16 ThingLimit = (Tslen*BitmapFontData.Width);

      for (uint16 Thing = 0; Thing < ThingLimit; Thing++) {

        // Get a glyph

        #define RealWidth ((BitmapFontData.Width + 7) / 8)
        #define RealPixelWidth (RealWidth * 8)

        char Character = TestString[Thing];
        if (Thing >= Tslen) break;

        uint64* Glyph = (uint64*)(GlyphData + (Character * BitmapFontData.GlyphSize) + (Thingy * RealWidth));
        uint64 ThisGlyph = *Glyph;

        // Draw said glyph

        for (uint32 Bit = 0; Bit < BitmapFontData.Width; Bit++) {

          #define Offset (RealPixelWidth - BitmapFontData.Width)

          #define ThisX ((Thing * BitmapFontData.Width) + (BitmapFontData.Width - Bit - 1) + StartWinSz[0] + (WindowSize[0] - ThingLimit) / 2)
          #define ThisY (Thingy + StartWinSz[1] + (WindowSize[1] - BitmapFontData.Height) / 2)

          if (Thing < Cutoff) {

            if ((ThisGlyph & (1ULL << (Bit+Offset))) != 0) {
              DrawPixel(0xFFFFFF, ThisX, ThisY);
            } else {
              DrawPixel(0, ThisX, ThisY);
            }

          } else {

            if ((ThisGlyph & (1ULL << (Bit+Offset))) != 0) {
              DrawPixel(0xFFFFFF, ThisX, ThisY);
            } else {
              DrawPixel(0x41BCCB, ThisX, ThisY);
            }

          }

        }

      }

    }

  }

  // (Depending on the system type, either wait for a keypress or just
  // stall the system for a while)

  if ((InfoTable->Firmware.Type == EfiFirmware) && (InfoTable->Firmware.Efi.SupportsConIn == true)) {

    // (Wait for a keypress)

    efiInputKey PhantomKey;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

  } else {

    // (Do a lot of NOPs to stall the system)

    for (auto Count = 0; Count < 500000000; Count++) {
      __asm__ __volatile__ ("nop");
    }

  }

  // (Return.)

  return;

}
