// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [64-bit Graphics.h - do not use with 16- or 32-bit code]
// [Relies on kernel- and platform-specific headers]

#ifndef SERRA_KERNEL_GRAPHICS_H
#define SERRA_KERNEL_GRAPHICS_H

  // Include standard and/or necessary headers.

  #include "../Stdint.h"
  #include "../../../Common.h"

  #include "Fonts/Fonts.h"

  // Include definitions used throughout the graphics subsystem.

  typedef enum _consoleType : uint8 {

    UnknownConsole = 0, // (Unknown or unsupported console)

    VgaConsole = 1, // (Uses framebuffer, 16-bit characters with ((Color << 8) | Char))
    EfiConsole = 2, // (Uses gST->ConOut, or efiSimpleTextOutputProtocol)

    GraphicalConsole = 3 // (Uses graphics mode, check for bitmap font.)

  } consoleType;

  typedef struct _consoleInfo {

    bool Supported; // (Does this system support a console?)

    void* Framebuffer; // (A pointer to the 'framebuffer', if used)
    consoleType Type; // (What type of console is this?)

    uint16 LimitX; // (Horizontal/X resolution, in *characters*)
    uint16 LimitY; // (Vertical/Y resolution, in *characters*)

    uint16 PosX; // (The current horizontal position)
    uint16 PosY; // (The current vertical position)

  } consoleInfo;

  typedef struct _graphicsColorInfo {

    uint8 Width; // (Width, in bits)
    uint8 Offset; // (Offset (shift left), in bits)

  } graphicsColorInfo;

  typedef struct _graphicsInfo {

    bool Supported; // (Does this system support graphics mode?)

    uint8 BytesPerPixel; // (Bytes per pixel)
    void* Framebuffer; // (A pointer to the framebuffer)
    uint64 Pitch; // (Bytes per row, including padding)

    uint16 LimitX; // (Horizontal/X resolution, in pixels)
    uint16 LimitY; // (Vertical/Y resolution, in pixels)

    graphicsColorInfo Colors[3]; // (Colors[0] is red, Colors[1] is green, Colors[2] is blue)

  } graphicsInfo;

  // Include functions and global variables from Console.c

  extern consoleInfo ConsoleInfo;
  bool InitializeConsole(commonInfoTable* Table);

  // Include functions and global variables from Graphics.c

  extern graphicsInfo GraphicsInfo;
  bool InitializeGraphics(commonInfoTable* Table);

#endif
