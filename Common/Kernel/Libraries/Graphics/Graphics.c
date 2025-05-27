// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../String.h"
#include "../../../Common.h"
#include "Graphics.h"

// (TODO - Variables)

graphicsData GraphicsData = {0};



// (TODO - A function to initialize the graphics subsystem)
// (This is required for the console subsystem later on.)

void InitializeGraphicsSubsystem(void* InfoTable) {

  // Before we do anything else, let's check that the pointer we were
  // given actually matches a commonInfoTable{} structure.

  commonInfoTable* Table = (commonInfoTable*)InfoTable;

  if (Table->Signature != commonInfoTableSignature) {
    return;
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

  return;

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

  if (CanExecuteOperation((PosX + Width), (PosY + Height)) == true) {

    // Now that we know we can safely draw this rectangle, let's do
    // exactly that!

    // (Allocate a buffer (from the stack) the size of one line)

    auto FbColor = TranslateRgbColorValue(Color);
    auto FbWidth = (GraphicsData.Bpp * Width);

    uint8 Buffer[FbWidth];

    // (Fill out the buffer, hopefully somewhat efficiently)

    uint16 Counter = 0;
    uint64 Data = FbColor;

    while (Counter < Width) {

      Memcpy((void*)&Buffer[GraphicsData.Bpp * Counter],
             (const void*)&Data,
             (uint64)GraphicsData.Bpp);

      Counter++;

    }

    // Now that we've filled out the buffer, we can copy it to the
    // framebuffer, one line at a time.

    uintptr Address = ((uintptr)GraphicsData.Framebuffer
                       + (GraphicsData.Bpp * PosX)
                       + (GraphicsData.Pitch * PosY));

    for (auto Line = 0; Line < Height; Line++) {

      Memcpy((void*)Address, (const void*)Buffer, FbWidth);
      Address += GraphicsData.Pitch; // Move to the next line

    }

  }

  // Return.

  return;

}
