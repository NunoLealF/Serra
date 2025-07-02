// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "../Libraries/String.h"
#include "../Memory/Memory.h"
#include "../../Common.h"
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

  // Next, let's see if the memory management subsystem is enabled; we
  // need it to be, in order to implement double buffering support.

  if (MmSubsystemData.IsEnabled == false) {
    return false;
  }

  // After that, let's check to see if we're in a graphics mode or not.

  switch (Table->Display.Type) {

    case DisplayType_Vbe:
      [[fallthrough]];

    case DisplayType_Gop:
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

    // Finally, let's go ahead and initialize double buffering (by
    // allocating space for GraphicsData.Buffer)

    const uintptr Size = (GraphicsData.LimitY * GraphicsData.Pitch);
    void* Buffer = Allocate(&Size);

    // (If we weren't able to allocate it, then return a null pointer;
    // otherwise, clear it.)

    if (Buffer != NULL) {
      Memset(Buffer, 0, Size);
      GraphicsData.Buffer = Buffer;
    } else {
      return false;
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
  } else if ((PosX > GraphicsData.LimitX) || (PosY > GraphicsData.LimitY)) {
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

    // Now that we know we are, let's calculate the specific offset
    // within the framebuffer we need to write to, as well as the
    // correct color value.

    auto FbColor = TranslateRgbColorValue(Color);
    uintptr Offset = ((GraphicsData.Bpp * PosX) + (GraphicsData.Pitch * PosY));

    // Next, let's copy it to the double buffer - as well as the
    // framebuffer - using Memcpy(). This isn't really the most
    // efficient solution, but it *should* work

    void* Backbuffer = (void*)((uintptr)GraphicsData.Buffer + Offset);
    void* Framebuffer = (void*)((uintptr)GraphicsData.Framebuffer + Offset);

    Memcpy(Backbuffer, (const void*)&FbColor, GraphicsData.Bpp);
    Memcpy(Framebuffer, (const void*)Backbuffer, GraphicsData.Bpp);

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

    // Now that we know we can safely draw this without going out of
    // bounds, let's do exactly that:

    // (Allocate a buffer (from the stack) the size of one line)

    auto FbWidth = (GraphicsData.Bpp * Width) + (GraphicsData.Bpp % 2);
    uint8 Buffer[FbWidth];

    // (Fill out the buffer using `MemsetBlock()`, hopefully
    // somewhat efficiently)

    auto FbColor = TranslateRgbColorValue(Color);
    MemsetBlock(Buffer, &FbColor, FbWidth, GraphicsData.Bpp);

    // Now that we've filled out the buffer, we can copy it to the
    // framebuffer, one line at a time.

    // (Since we use double buffering, we first copy to our back
    // buffer, and *then* we copy it to the framebuffer.)

    uintptr Offset = ((GraphicsData.Bpp * PosX) + (GraphicsData.Pitch * PosY));

    void* Backbuffer = (void*)((uintptr)GraphicsData.Buffer + Offset);
    void* Framebuffer = (void*)((uintptr)GraphicsData.Framebuffer + Offset);

    // (Copy line by line to the back buffer)

    for (auto Line = 0; Line < Height; Line++) {

      Memcpy((void*)((uintptr)Backbuffer + (GraphicsData.Pitch * Line)),
             (const void*)Buffer,
             FbWidth);

    }

    // (Copy everything (from the start to the end of the rectangle) from
    // the back buffer to the framebuffer in one move)

    Memcpy(Framebuffer, (const void*)Backbuffer,
          ((GraphicsData.Bpp * Width) + (GraphicsData.Pitch * Height)));

  }

  // Return.

  return;

}



// (TODO - A function that draws a bitmap font)

// As with PSF2 bitmaps, 'Width' is padded out as necessary; for example,
// a 5-bit bitmap is drawn as if it were an 8-bit one

void DrawBitmapFont(const char* String, const bitmapFontData* Font, bool UpdateFramebuffer, uint32 ForegroundColor, uint32 BackgroundColor, uint16 PosX, uint16 PosY) {

  // First, let's calculate the length of the string we were given.

  auto Length = Strlen(String);

  // Second, let's make sure that the pointer to the bitmap font is
  // even valid in the first place.

  if (Font != NULL) {

    // Finally, let's make sure that we're in a graphics mode, and that
    // we won't draw anywhere "out of bounds".

    uint16 LimitX = (PosX + (Font->Width * Length));
    uint16 LimitY = (PosY + Font->Height);

    if (CanExecuteOperation(LimitX, LimitY) == true) {

      // Okay - now that we know that everything appears to be valid,
      // we can start drawing our bitmap font.

      // Let's start by creating a lookup table for each possible 8-bit
      // combination (0b00000000 through 0b11111111), with our preferred
      // foreground color:

      // (Pre-process the framebuffer color values for `ForegroundColor`
      // and `BackgroundColor`)

      auto BgColor = TranslateRgbColorValue(BackgroundColor);
      auto FgColor = TranslateRgbColorValue(ForegroundColor);

      // (Allocate space for our lookup table, and fill it out as necessary;
      // in order to improve performance, we also cache it, and use a
      // different process for some color combinations)

      static uint32 CachedColors[2] = {0};
      static uint8 BitmapLut[256][8 * sizeof(uint64)];

      #define GetBitmapLutPosition(Byte, Bit) ((void*)&BitmapLut[Byte][GraphicsData.Bpp * Bit])

      if ((CachedColors[0] != BgColor) || (CachedColors[1] != FgColor)) {

        // (If the background and foreground colors are both black or
        // white, then skip the process entirely)

        if ((BackgroundColor == 0) && (ForegroundColor == 0)) {

          Memset(GetBitmapLutPosition(0, 0), 0, (128 * 8 * sizeof(uint64)));

        } else if ((BackgroundColor == 0xFFFFFF) && (ForegroundColor == 0xFFFFFF)) {

          Memset(GetBitmapLutPosition(0, 0), 0xFF, (128 * 8 * sizeof(uint64)));

        } else {

          // (Otherwise, let's build up our lookup table.)

          for (auto Byte = 0; Byte < 256; Byte++) {

            // (Handle each individual bit)

            auto Bit = 8;
            auto Complement = 0;

            while (Bit > 0) {

              // Depending on whether this bit is set, either draw the
              // foreground or the background color.

              if ((Byte & (1ULL << --Bit)) != 0) {
                *(uint64*)GetBitmapLutPosition(Byte, Complement++) = FgColor;
              } else {
                *(uint64*)GetBitmapLutPosition(Byte, Complement++) = BgColor;
              }

            }

          }

        }

        // (Update `CachedColors` - this lets us avoid rebuilding the
        // lookup tables if we already used the same colors last time)

        CachedColors[0] = BgColor;
        CachedColors[1] = FgColor;

      }

      // Now that we're done, let's pre-process a few commonly
      // used variables for performance:

      // (1) The difference between the actual and padded font width,
      // as well as the dimensions of the font;

      auto Height = Font->Height;
      auto Width = Font->Width;

      auto PaddedWidth = ((Width + 7) / 8);

      // (2) The initial offset, as well as the addresses of the back
      // and front buffers (`Backbuffer` and `Framebuffer`).

      const uintptr Offset = ((PosX * GraphicsData.Bpp) + (PosY * GraphicsData.Pitch));

      const uintptr Backbuffer = ((uintptr)GraphicsData.Buffer + Offset);
      const uintptr Framebuffer = ((uintptr)GraphicsData.Framebuffer + Offset);

      // Finally, we can start drawing to the back buffer, like this:

      uintptr Address = Backbuffer;

      for (uint32 Line = 0; Line < Height; Line++) {

        // (For each character within our string..)

        for (auto Index = 0; Index < Length; Index++) {

          // (Obtain the character, and from there, the bitmap specified
          // by the font for that character)

          char Character = String[Index];

          uint8 Bitmap = *(uint8*)((uintptr)Font->GlyphData
                                   + (Font->GlyphSize * Character)
                                   + (Line * PaddedWidth));

          // (Copy over the necessary pixels for that specific byte
          // from the lookup table we made earlier)

          const void* Pixels = GetBitmapLutPosition(Bitmap, 0);
          Memcpy((void*)Address, Pixels, (GraphicsData.Bpp * Width));

          // (Increment `Address` to move onto the next character)

          Address += (GraphicsData.Bpp * Width);

        }

        // (Unless we're at the very last line, move `Address` to point
        // to the next line)

        if ((Line + 1) < Height) {

          Address += GraphicsData.Pitch;
          Address -= (GraphicsData.Bpp * Width * Length);

        }

      }

      // Now that we've finished drawing to the back buffer, we can
      // finally copy everything to the framebuffer in one go.

      if (UpdateFramebuffer == true) {

        // (Calculate the distance between the start (`Backbuffer`) and
        // the end (`Address`) of the text we just finished drawing)

        uintptr Delta = (Address - Backbuffer);

        // (Copy from the back buffer to the framebuffer, using Memcpy())

        Memcpy((void*)Framebuffer, (const void*)Backbuffer, (uint64)Delta);

      }

    }

  }

  // Return.

  return;

}
