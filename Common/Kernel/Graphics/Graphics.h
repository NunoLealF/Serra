// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_GRAPHICS_H
#define SERRA_KERNEL_GRAPHICS_H

  // Include standard and/or necessary headers.

  #include "../Libraries/Stdint.h"
  #include "Console/Console.h"
  #include "Fonts/Fonts.h"

  // Include definitions used throughout the graphics subsystem.

  typedef struct _graphicsSubsystemData {

    // (Mode information)

    bool IsSupported; // (Does this system support graphics mode?)

    uint8 Bpp; // (Bytes per pixel (not bits!); .Bits.PerPixel / 8)
    uint64 Pitch; // (Bytes per row, including padding)

    uint16 LimitX; // (Horizontal/X resolution, in pixels)
    uint16 LimitY; // (Vertical/Y resolution, in pixels)

    // (Framebuffer and double buffering information)

    void* Framebuffer; // (A pointer to the framebuffer)
    void* Buffer; // (A pointer to the double buffer)

    // (Color information; [0] is red, [1] is green, [2] is blue)

    struct {

      uint8 Width; // (Width, in bits)
      uint8 Offset; // (Offset (shift left), in bits)

      uint64 Lut[256]; // (Lookup table, for intensities 0-255)

    } Colors[3];

  } graphicsSubsystemData;

  // Include functions and global variables from Graphics.c

  extern graphicsSubsystemData GraphicsData;
  bool InitializeGraphicsSubsystem(void* InfoTable);

  void DrawPixel(uint32 Color, uint16 PosX, uint16 PosY);
  void DrawRectangle(uint32 Color, uint16 PosX, uint16 PosY, uint16 Width, uint16 Height);
  void DrawBitmapFont(const char* String, const bitmapFontData* Font, uint32 ForegroundColor, uint32 BackgroundColor, uint16 PosX, uint16 PosY);

#endif
