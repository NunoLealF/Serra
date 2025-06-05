// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "Fonts.h"

// (TODO: Font, uses #embed; in PSF1 *or* PSF2 format)
// (TODO: Also, global variables)

// (WARNING: Must be PSF1, or a PSF2 font with *fixed* 8 pixels width;
// it's not that I can't implement anything wider, but it's too difficult)

const uint8 BitmapFont[] = {
  #embed "Font.psf" if_empty('\0')
};

bitmapFontData BitmapFontData = {0};



// TODO: A function that checks whether BitmapFont[] is valid (returning
// true or false), and initializes BitmapFontData.

bool InitializeBitmapFont(void) {

  // Try to figure out what the font type is (PSF1 or PSF2), and depending
  // on the type, fill out BitmapFontData.

  if (((const psf1Header*)BitmapFont)->Signature == psf1HeaderSignature) {

    // We've found a PSF1 bitmap font, which is always 8 pixels wide, and
    // which has support for either 256 or 512 glyphs (+ Unicode table).

    // (Define variables)

    const psf1Header* Header = (const psf1Header*)BitmapFont;

    // (Fill out BitmapFontData{})

    BitmapFontData.IsSupported = true;
    BitmapFontData.Header = (const void*)Header;
    BitmapFontData.Type = Psf1Bitmap;

    if ((Header->HdrFlags & psf1HeaderHasUnicodeTable) != 0) {
      BitmapFontData.HasUnicodeTable = true;
    } else {
      BitmapFontData.HasUnicodeTable = false;
    }

    BitmapFontData.GlyphData = (const void*)((uintptr)BitmapFont + sizeof(psf1Header));
    BitmapFontData.GlyphSize = Header->Size;

    if ((Header->HdrFlags & psf1HeaderHas512Glyphs) != 0) {
      BitmapFontData.NumGlyphs = 512;
    } else {
      BitmapFontData.NumGlyphs = 256;
    }

    BitmapFontData.Width = 8;
    BitmapFontData.Height = Header->Size;

    // (Return true, to indicate that we found a valid bitmap font)

    return true;

  } else if (((const psf2Header*)BitmapFont)->Signature == psf2HeaderSignature) {

    // We've found a PSF2 bitmap font, which can be any number of pixels
    // wide (although always padded to 8-bit boundaries), and which has
    // support for an arbitrary amount of glyphs (+ Unicode table).

    // (Define variables, and check that the header appears to be valid)

    const psf2Header* Header = (const psf2Header*)BitmapFont;

    if (Header->HdrSize < sizeof(psf2Header)) {
      return false;
    } else if (Header->NumGlyphs < 256) {
      return false;
    } else if (Header->Width != 8) {
      return false;
    }

    // (Fill out BitmapFontData{})

    BitmapFontData.IsSupported = true;
    BitmapFontData.Header = (const void*)Header;
    BitmapFontData.Type = Psf2Bitmap;

    if ((Header->HdrFlags & psf2HeaderHasUnicodeTable) != 0) {
      BitmapFontData.HasUnicodeTable = true;
    } else {
      BitmapFontData.HasUnicodeTable = false;
    }

    BitmapFontData.GlyphData = (const void*)((uintptr)BitmapFont + sizeof(psf2Header));
    BitmapFontData.GlyphSize = Header->Size;
    BitmapFontData.NumGlyphs = Header->NumGlyphs;

    BitmapFontData.Width = Header->Width;
    BitmapFontData.Height = Header->Height;

    // (Return true, to indicate that we found a valid bitmap font)

    return true;

  }

  // Return false, if we haven't been able to figure out what it is.

  return false;

}
