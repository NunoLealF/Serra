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

  // Include definitions and global variables from Fonts/Bitmap.c

  typedef enum _bitmapFontType : uint8 {

    UnknownBitmap = 0,
    Psf1Bitmap = 1,
    Psf2Bitmap = 2

  } bitmapFontType;

  typedef struct _bitmapFontData {

    const void* Header; // Pointer to the font header (can be psf1Header, psf2Header..)
    bitmapFontType Type; // The type of this bitmap font.
    bool HasUnicodeTable; // Whether this font has a Unicode table or not.

    const void* GlyphData; // A pointer to the start of the glyph data.
    uint32 GlyphSize; // The size of each glyph, in bytes.
    uint32 NumGlyphs; // The amount of glyphs in the bitmap font.

    uint32 Width; // The width of each glyph, in pixels.
    uint32 Height; // The height of each glyph, in pixels.

  } bitmapFontData;

  #define psf1HeaderSignature 0x0436
  #define psf1HeaderHas512Glyphs (1ULL << 0)
  #define psf1HeaderHasUnicodeTable (1ULL << 1)

  typedef struct _psf1Header {

    uint16 Signature; // (Must match `psf1HeaderSignature`)
    uint8 HdrFlags; // (Can be 0h; see flags starting with psf1HeaderHas...)

    uint8 Size; // (Number of bytes per glyph *and* glyph height in pixels)

  } __attribute__((packed)) psf1Header;

  #define psf2HeaderSignature 0x864AB572
  #define psf2HeaderHasUnicodeTable (1ULL << 0)

  typedef struct _psf2Header {

    uint32 Signature; // (Must match `psf2HeaderSignature`)

    uint32 HdrVersion; // (Usually 0h?)
    uint32 HdrSize; // (Must be at least sizeof(psfHeader))
    uint32 HdrFlags; // (Can be 0h; see flags starting with psf2HeaderHas...)

    uint32 NumGlyphs; // (Number of glyphs)
    uint32 Size; // (Number of bytes per glyph)

    uint32 Height; // (Glyph height, in pixels)
    uint32 Width; // (Glyph width, in pixels)

  } __attribute__((packed)) psf2Header;

  extern const uint8 BitmapFont[];
  extern bitmapFontData BitmapFontData;



  // Include functions from Fonts/Bitmap.c (TODO)

  bool InitializeBitmapFont(void);



  // Include definitions and global variables from Console.c

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

  extern consoleInfo ConsoleInfo;



  // Include functions from Console.c (TODO)

  bool InitializeConsole(commonInfoTable* Table);



  // Include definitions and global variables from Graphics.c

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

  extern graphicsInfo GraphicsInfo;



  // Include functions from Graphics.c (TODO)

  bool InitializeGraphics(commonInfoTable* Table);

#endif
