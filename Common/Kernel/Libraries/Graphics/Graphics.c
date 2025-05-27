// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
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

      GraphicsData.Colors[Index].Offset++; // (Needed)

      while ((ColorMasks[Index] & (1ULL << Bit++)) != 0) {
        GraphicsData.Colors[Index].Width++;
      }

      // (Calculate lookup tables for each color mask as well; this
      // saves processing time later on)

      const uint64 WidthMask = (1ULL << GraphicsData.Colors[Index].Width) - 1;

      for (auto Intensity = 0; Intensity < 256; Intensity++) {
        GraphicsData.Colors[Index].Lut[Intensity] = (((WidthMask * Intensity) / 256) << GraphicsData.Colors[Index].Offset);
      }

    }

  }

  // Return - the caller can check whether the graphics subsystem was
  // initialized correctly by checking GraphicsData.IsSupported.

  return;

}



// (TODO - A function that translates an RGB color value into one accepted
// by the framebuffer)

uint64 TranslateRgbColorValue(uint32 Color) [[reproducible]] {

  // This uses the lookup tables we computed earlier (in GraphicsData{})
  // to help speed things up.

  return (GraphicsData.Colors[0].Lut[(Color >> 16) & 0xFF]
          | GraphicsData.Colors[1].Lut[(Color >> 8) & 0xFF]
          | GraphicsData.Colors[2].Lut[(Color >> 0) & 0xFF]);

}



// (TODO - A function to write a pixel)
