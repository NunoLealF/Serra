// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    for (auto Y = 0; Y < (InfoTable->Display.Graphics.LimitY); Y++) {

      // (Get necessary addresses and values)

      auto Buffer = (InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * Y));
      auto Size = (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 8);

      // (Use memset to display a pretty gradient)

      Memset((void*)Buffer, (uint8)(Y * 255 / InfoTable->Display.Graphics.LimitY), Size);

    }

    // (Display font characters)
    // (WARNING: Fonts must not exceed 8px width with initial driver (!))

    uintptr GlyphData = (uintptr)BitmapFontData.GlyphData;

    const char* TestString = "Hi, this is graphics-mode Serra! <3 May 26 2025"; // [48]

    for (uint16 Thingy = 0; Thingy < BitmapFontData.Height; Thingy++) {

      for (uint16 Thing = 0; Thing < (InfoTable->Display.Graphics.LimitX / BitmapFontData.Width); Thing++) {

        // Get a glyph

        #define RealWidth ((BitmapFontData.Width + 7) / 8)
        #define RealPixelWidth (RealWidth * 8)

        char Character = TestString[Thing];
        if (Thing >= 47) break;

        uint64* Glyph = (uint64*)(GlyphData + (Character * BitmapFontData.GlyphSize) + (Thingy * RealWidth));
        uint64 ThisGlyph = *Glyph;

        // Get colors for that glyph

        uint64 EnabledMask_s1 = TranslateRgbColorValue(0xFFFFFF);
        uint64 DisabledMask_s1 = TranslateRgbColorValue(0x000000);

        uint64 EnabledMask_s2 = TranslateRgbColorValue(0xFFFFFF);
        uint64 DisabledMask_s2 = TranslateRgbColorValue(0x41BCCB);

        // Draw that glyph

        for (uint32 Bit = 0; Bit < BitmapFontData.Width; Bit++) {

          auto Size = (InfoTable->Display.Graphics.Bits.PerPixel / 8);
          uint64 Ptr = (InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * Thingy) + (Size * ((Thing * BitmapFontData.Width) + (BitmapFontData.Width - 1 - Bit))));

          #define Offset (RealPixelWidth - BitmapFontData.Width)

          if (Thing < 36) {

            if ((ThisGlyph & (1ULL << (Bit+Offset))) != 0) {
              Memcpy((void*)Ptr, (const void*)&EnabledMask_s1, Size);
            } else {
              Memcpy((void*)Ptr, (const void*)&DisabledMask_s1, Size);
            }

          } else {

            if ((ThisGlyph & (1ULL << (Bit+Offset))) != 0) {
              Memcpy((void*)Ptr, (const void*)&EnabledMask_s2, Size);
            } else {
              Memcpy((void*)Ptr, (const void*)&DisabledMask_s2, Size);
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
