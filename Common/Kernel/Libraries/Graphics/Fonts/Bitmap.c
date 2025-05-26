// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../Graphics.h"

// (TODO: Font, uses #embed; in PSF1 *or* PSF2 format)

const uint8 BitmapFont[] = {
  #embed "Font.psf"
};



// (TODO: Something to validate the PSF header.. this should be a
// constructor somewhere)

bool BitmapFontHasPsfHeader(void) {

  // Check if the font has a PSF1 or PS2 header, and if so, return true;
  // otherwise, return false.

  uint16 PossiblePsf1Signature = *(uint16*)(BitmapFont);
  uint32 PossiblePsf2Signature = *(uint32*)(BitmapFont);

  if (PossiblePsf1Signature == psf1HeaderSignature) {
    return true;
  } else if (PossiblePsf2Signature == psf2HeaderSignature) {
    return true;
  }

  return false;

}



// (TODO: Something to read PSF files; in theory, this shouldn't be too,
// hard, I hope)
