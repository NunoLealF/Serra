// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [64-bit Graphics.h - do not use with 16- or 32-bit code]
// [Relies on kernel- and platform-specific headers]

#ifndef SERRA_KERNEL_GRAPHICS_H
#define SERRA_KERNEL_GRAPHICS_H

  // Include standard and/or necessary headers.

  #include "../Stdint.h"
  #include "../../Constructors/System/System.h"

  // Include functions from Graphics.c (TODO)

  // Include definitions used in Fonts/Bitmap.c (TODO)

  #define psf1HeaderSignature 0x0436
  #define psf2HeaderSignature 0x864AB572

  #define psf1HeaderHas512Glyphs (1ULL << 0)
  #define psf1HeaderHasUnicodeTable (1ULL << 1)
  #define psf2HeaderHasUnicodeTable (1ULL << 0)

  typedef struct __psf1Header {

    uint16 Signature; // (Must match `psf1HeaderSignature`)
    uint8 HdrFlags; // (Can be 0h; see flags starting with psf1HeaderHas...)

    uint8 Size; // (Number of bytes per glyph *and* glyph height in pixels)

  } __attribute__((packed)) psf1Header;

  typedef struct __psf2Header {

    uint32 Signature; // (Must match `psf2HeaderSignature`)

    uint32 HdrVersion; // (Usually 0h?)
    uint32 HdrSize; // (Must be at least sizeof(psfHeader))
    uint32 HdrFlags; // (Can be 0h; see flags starting with psf2HeaderHas...)

    uint32 NumGlyphs; // (Number of glyphs)
    uint32 Size; // (Number of bytes per glyph)

    uint32 Height; // (Glyph height, in pixels)
    uint32 Width; // (Glyph width, in pixels)

  } __attribute__((packed)) psf2Header;

  // Include functions from Fonts/Bitmap.c (TODO)

#endif
