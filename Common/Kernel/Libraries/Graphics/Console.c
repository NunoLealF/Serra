// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../../../Common.h"
#include "Graphics.h"

// (TODO - Some sort of common.. console subsystem, maybe? I just need a
// printf that will work nearly universally, I guess)

// (TODO - This is the constructor, so it *prepares* the environment for
// ../../Libraries/Graphics/*)

consoleInfo ConsoleInfo = {0};

// (TODO - Return value is "is console supported?")

bool InitializeConsole(commonInfoTable* Table) {

  // Fill out ConsoleInfo{}, depending on the text/graphics mode we're in.

  if (Table->Display.Type == UnknownDisplay) {

    // (If we don't have a display attached, then no need to fill out
    // the table.)

    ConsoleInfo.Supported = false;
    ConsoleInfo.Type = UnknownConsole;

  } else if (Table->Display.Type == VgaDisplay) {

    // (If we're on VGA text mode, then we have a hardware-provided
    // framebuffer at B8000h, which is at least 80*25 characters wide.)

    ConsoleInfo.Supported = true;
    ConsoleInfo.Framebuffer = (void*)0xB8000;
    ConsoleInfo.Type = VgaConsole;

    ConsoleInfo.LimitX = Table->Display.Text.LimitX;
    ConsoleInfo.LimitY = Table->Display.Text.LimitY;

    ConsoleInfo.PosX = Table->Display.Text.PosX;
    ConsoleInfo.PosY = Table->Display.Text.PosY;

  } else if (Table->Display.Type == EfiTextDisplay) {

    // (If we're on EFI text mode, then that means we have to use the
    // EFI Simple Text Output Protocol at gST->ConOut; we also don't
    // have to keep track of the position.)

    ConsoleInfo.Supported = true;
    ConsoleInfo.Type = EfiConsole;

    ConsoleInfo.LimitX = Table->Display.Text.LimitX;
    ConsoleInfo.LimitY = Table->Display.Text.LimitY;

  } else if ((Table->Display.Type == VbeDisplay) || (Table->Display.Type == GopDisplay)) {

    // (If we're in a graphics mode, then we have to provide our own
    // bitmap font, so we'll fill out ConsoleInfo{} based on the values
    // provided in BitmapFontData{})

    // (If we're using a graphics mode, then that means we have to
    // provide a console ourselves, so let's initialize the bitmap
    // font in Fonts/Font.psf)

    if ((BitmapFontData.Width == 0) || (BitmapFontData.Height == 0)) {

      ConsoleInfo.Supported = false;
      return false;

    }

    // (Now that we have, let's calculate the necessary values for
    // ConsoleInfo{})

    ConsoleInfo.Supported = true;
    ConsoleInfo.Framebuffer = Table->Display.Graphics.Framebuffer.Pointer;
    ConsoleInfo.Type = GraphicalConsole;

    ConsoleInfo.LimitX = (Table->Display.Graphics.LimitX / BitmapFontData.Width);
    ConsoleInfo.LimitY = (Table->Display.Graphics.LimitY / BitmapFontData.Height);

  }

  // Return a value showing whether the console is supported

  return ConsoleInfo.Supported;
}
