// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../String.h"
#include "../../../Common.h"
#include "Graphics.h"

// (TODO - Variables)

graphicsSubsystemData GraphicsData = {0};



// (TODO - A function to initialize the graphics subsystem)
// (This is required for the console subsystem later on.)

// Returns true if okay, false if not.

bool InitializeGraphicsSubsystem(void* InfoTable) {

  // Before we do anything else, let's check that the pointer we were
  // given actually matches a commonInfoTable{} structure.

  commonInfoTable* Table = (commonInfoTable*)InfoTable;

  if (Table->Signature != commonInfoTableSignature) {
    return false;
  }

  // Next, let's check to see if we're in a graphics mode or not.

  switch (Table->Display.Type) {

    case VbeDisplay:
      [[fallthrough]];

    case GopDisplay:
      GraphicsData.IsSupported = true;
      break;

    default:
      GraphicsData.IsSupported = false;
      break;

  }

  // If we are, then initialize GraphicsData{} with the necessary
  // values.

  if (GraphicsData.IsSupported == true) {

    // (Fill in basic mode information)

    GraphicsData.Bpp = (Table->Display.Graphics.Bits.PerPixel / 8);
    GraphicsData.Framebuffer = Table->Display.Graphics.Framebuffer.Pointer;
    GraphicsData.Pitch = Table->Display.Graphics.Pitch;

    GraphicsData.LimitX = Table->Display.Graphics.LimitX;
    GraphicsData.LimitY = Table->Display.Graphics.LimitY;

    // (Fill in color information - this is slightly more complicated,
    // as we're converting from masks to width+offset)

    uint64 ColorMasks[3] = {Table->Display.Graphics.Bits.RedMask,
                            Table->Display.Graphics.Bits.GreenMask,
                            Table->Display.Graphics.Bits.BlueMask};

    for (auto Index = 0; Index < 3; Index++) {

      // (Calculate the offset (how many bits to shift left by) and
      // width (how many bits per color) for each color mask)

      auto Bit = 0;

      while ((ColorMasks[Index] & (1ULL << Bit++)) == 0) {
        GraphicsData.Colors[Index].Offset++;
      }

      GraphicsData.Colors[Index].Width++; // (This is necessary)

      while ((ColorMasks[Index] & (1ULL << Bit++)) != 0) {
        GraphicsData.Colors[Index].Width++;
      }

      // (Calculate lookup tables for each color mask as well; this
      // saves processing time later on)

      const uint64 WidthMask = (1ULL << GraphicsData.Colors[Index].Width) - 1;

      for (auto Intensity = 0; Intensity < 256; Intensity++) {
        GraphicsData.Colors[Index].Lut[Intensity] = (((WidthMask * Intensity) / 255) << GraphicsData.Colors[Index].Offset);
      }

    }

  }

  // Return - the caller can check whether the graphics subsystem was
  // initialized correctly by checking GraphicsData.IsSupported.

  return true;

}



// (TODO - A function that checks whether we're within bounds *and* that
// whatever we're trying to draw to is within bounds)

static inline bool CanExecuteOperation(uint16 PosX, uint16 PosY) {

  if (GraphicsData.IsSupported == false) {
    return false;
  } else if ((PosX >= GraphicsData.LimitX) || (PosY >= GraphicsData.LimitY)) {
    return false;
  }

  return true;

}



// (TODO - A static function that translates an RGB color value into one
// accepted by the framebuffer)

static inline uint64 TranslateRgbColorValue(uint32 Color) [[reproducible]] {

  // This uses the lookup tables we computed earlier (in GraphicsData{})
  // to help speed things up.

  return (GraphicsData.Colors[0].Lut[(Color >> 16) & 0xFF]
          | GraphicsData.Colors[1].Lut[(Color >> 8) & 0xFF]
          | GraphicsData.Colors[2].Lut[(Color >> 0) & 0xFF]);

}



// (TODO - A function to put a pixel on the screen)

// You probably shouldn't use this for anything 'large', this is just for
// single pixels and demos which really can't afford anything more.

void DrawPixel(uint32 Color, uint16 PosX, uint16 PosY) {

  // First, let's make sure that we're in a graphics mode, and that
  // the positions we were given don't go out of bounds.

  if (CanExecuteOperation(PosX, PosY) == true) {

    // Now that we know we are, let's calculate the specific address
    // within the framebuffer we need to write to

    uintptr Address = ((uintptr)GraphicsData.Framebuffer
                       + (GraphicsData.Bpp * PosX)
                       + (GraphicsData.Pitch * PosY));

    // Copy it to the framebuffer using Memcpy() - this is *not* the
    // most efficient solution, but it'll work fine for simple
    // operations.

    auto FbColor = TranslateRgbColorValue(Color);
    Memcpy((void*)Address, (const void*)&FbColor, GraphicsData.Bpp);

  }

  // Return.

  return;

}



// (TODO - A function to put a bitmap on the screen)

void DrawRectangle(uint32 Color, uint16 PosX, uint16 PosY, uint16 Width, uint16 Height) {

  // First, let's make sure that we're in a graphics mode, and that
  // we won't draw anywhere 'out of bounds'.

  uint16 LimitX = (PosX + Width);
  uint16 LimitY = (PosY + Height);

  if (CanExecuteOperation(LimitX, LimitY) == true) {

    // Now that we know we can safely draw this rectangle, let's do
    // exactly that!

    // (Allocate a buffer (from the stack) the size of one line)

    auto FbWidth = (GraphicsData.Bpp * Width) + (GraphicsData.Bpp % 2);
    uint8 Buffer[FbWidth];

    // (Fill out the buffer, hopefully somewhat efficiently)

    auto FbColor = TranslateRgbColorValue(Color);
    MemsetBlock(Buffer, &FbColor, FbWidth, GraphicsData.Bpp);

    // Now that we've filled out the buffer, we can copy it to the
    // framebuffer, one line at a time.

    uintptr Framebuffer = ((uintptr)GraphicsData.Framebuffer
                            + (GraphicsData.Bpp * PosX)
                            + (GraphicsData.Pitch * PosY));

    for (auto Line = 0; Line < Height; Line++) {

      Memcpy((void*)Framebuffer, (const void*)Buffer, FbWidth);
      Framebuffer += GraphicsData.Pitch; // Move onto the next line

    }

  }

  // Return.

  return;

}



// (TODO - A function that draws a bitmap font, with optional transparency)

// (WARNING - This is a good start, hopefully.. but it's only really
// suitable for large operations, certainly not individual letters)

// As with PSF2 bitmaps, 'Width' is padded out as necessary; for example,
// a 5-bit bitmap is drawn as if it were an 8-bit one

void DrawBitmapFont(const char* String, const bitmapFontData* Font, uint32 ForegroundColor, [[maybe_unused]] uint32 BackgroundColor, bool UseTransparency, uint16 PosX, uint16 PosY) {

  // First, let's calculate the length of the string we were given.

  auto Length = Strlen(String);

  // Second, let's make sure that the pointer to the bitmap font is
  // even valid in the first place.

  if (Font != NULL) {

    // Finally, let's make sure that we're in a graphics mode, and that
    // we won't draw anywhere "out of bounds".

    uint16 LimitX = (PosX + (Font->Width * Length));
    uint16 LimitY = (PosY + (Font->Height));

    if (CanExecuteOperation(LimitX, LimitY) == true) {

      // Okay - now that we know that everything appears to be valid,
      // we can start drawing our bitmap Font->

      // Let's start by allocating a buffer (the size of one line)
      // from the stack, that we'll update as needed.

      auto FbWidth = (GraphicsData.Bpp * (Font->Width * Length)) + (GraphicsData.Bpp % 2);
      uint8 Buffer[FbWidth];

      // Next, if we aren't using transparency, we also want to create
      // a "background buffer" that we can copy from each time.

      // (In this case, filling a buffer is more expensive than copying
      // it, so this can speed up the function in the long run)

      [[maybe_unused]] uint8 BackgroundBuffer[FbWidth];

      if (UseTransparency == false) {

        uint64 Pixel = TranslateRgbColorValue(BackgroundColor);
        MemsetBlock(BackgroundBuffer, &Pixel, FbWidth, GraphicsData.Bpp);

      }

      // We also want to preprocess a few things for performance; namely:

      // (1) The pointers to each of the characters within the bitmap
      // font (we'll need this either way, why not do it now?);

      void* CharacterList[Length];

      for (auto Index = 0; Index < Length; Index++) {

        CharacterList[Index] = (void*)((uintptr)Font->GlyphData + (Font->GlyphSize * String[Index]));

      }

      // (2) The address we need to copy to (and possibly, from).

      uintptr Framebuffer = ((uintptr)GraphicsData.Framebuffer
                              + (GraphicsData.Bpp * PosX)
                              + (GraphicsData.Pitch * PosY));

      // Finally, let's deal with each individual line:

      for (uint32 Line = 0; Line < Font->Height; Line++) {

        // Depending on whether we're using transparency or not, either
        // copy the background from the framebuffer, or the background
        // buffer we defined earlier on.

        if (UseTransparency == true) {
          Memcpy((void*)Buffer, (const void*)Framebuffer, FbWidth);
        } else {
          Memcpy((void*)Buffer, (const void*)BackgroundBuffer, FbWidth);
        }

        // (Calculate the difference between the 'real' font width,
        // and the number of bits (which is rounded up to 8))

        uint32 ByteWidth = ((Font->Width + 7) / 8);
        uint32 FontWidth = Font->Width;

        uint32 WidthDifference = ((ByteWidth * 8) - FontWidth);

        // Read through each bit of the bitmap for each character of
        // our string, like this:

        uint64 Address = (uint64)Buffer;

        // (For each character of our string..)

        for (auto Index = 0; Index < Length; Index++) {

          uint8 Character = *(uint8*)((uintptr)CharacterList[Index] + (Line * ByteWidth)) >> WidthDifference;

          // (For each bit in our bitmap.. (we iterate in reverse order))

          auto Bit = FontWidth;

          while (Bit > 0) {

            // (If this bit is set, then write it to our line buffer)

            if ((Character & (1ULL << --Bit)) != 0) {

              uint64 Pixel = TranslateRgbColorValue(ForegroundColor);
              Memcpy((void*)Address, &Pixel, GraphicsData.Bpp);

            }

            // (Increment Address)

            Address += GraphicsData.Bpp;

          }

        }

        // Copy our line buffer to the framebuffer, and move our
        // framebuffer pointer to the next line over.

        Memcpy((void*)Framebuffer, (const void*)Buffer, FbWidth);
        Framebuffer += GraphicsData.Pitch;

      }

    }

  }

  // Return.

  return;

}
