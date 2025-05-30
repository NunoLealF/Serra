// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_GRAPHICS_FONTS_H
#define SERRA_KERNEL_GRAPHICS_FONTS_H

  // Include standard headers.

  #include "../../Libraries/Stdint.h"

  // Include definitions used in Bitmap.c

  typedef enum _bitmapFontType : uint8 {

    UnknownBitmap = 0,
    Psf1Bitmap = 1,
    Psf2Bitmap = 2

  } bitmapFontType;

  typedef struct _bitmapFontData {

    bool IsSupported; // (Whether InitializeBitmapFont() has been called yet)

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

  // Include functions and global variables from Bitmap.c

  extern const uint8 BitmapFont[];
  extern bitmapFontData BitmapFontData;

  bool InitializeBitmapFont(void);

#endif
