// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../../Common.h"
#include "Graphics.h"

// (TODO - This is the constructor, so it *prepares* the environment for
// ../../Libraries/Graphics/*)

// (TODO: Initialize GraphicsInfo)

graphicsInfo GraphicsInfo = {0};

// (TODO - return value is "is graphics mode supported?")

bool InitializeGraphics(commonInfoTable* Table) {

  // Check whether we're in a graphics mode or not

  switch (Table->Display.Type) {

    case VbeDisplay:
      [[fallthrough]];

    case GopDisplay:
      GraphicsInfo.Supported = true;
      break;

    default:
      GraphicsInfo.Supported = false;
      break;

  }

  // If we are, then initialize GraphicsInfo{}; otherwise, return false

  if (GraphicsInfo.Supported == true) {

    GraphicsInfo.BytesPerPixel = (Table->Display.Graphics.Bits.PerPixel / 8);
    GraphicsInfo.Framebuffer = (Table->Display.Graphics.Framebuffer.Pointer);
    GraphicsInfo.Pitch = (Table->Display.Graphics.Pitch);

    GraphicsInfo.LimitX = (Table->Display.Graphics.LimitX);
    GraphicsInfo.LimitY = (Table->Display.Graphics.LimitY);

    // (For each color mask..)

    uint64 Colors[3] = {Table->Display.Graphics.Bits.RedMask,
                        Table->Display.Graphics.Bits.GreenMask,
                        Table->Display.Graphics.Bits.BlueMask};

    for (auto Index = 0; Index < 3; Index++) {

      while ((Colors[Index] & 1) == 0) {

        GraphicsInfo.Colors[Index].Offset++;
        Colors[Index] >>= 1;

      }

      while ((Colors[Index] & 1) == 1) {

        GraphicsInfo.Colors[Index].Width++;
        Colors[Index] >>= 1;

      }

    }

    // (Return true, since we did find a graphics mode)

    return true;

  } else {

    return false;

  }

}
