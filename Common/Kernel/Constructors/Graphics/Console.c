// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "Graphics.h"


// (TODO: Something to validate the PSF header.. this should be a
// constructor somewhere)

bool CheckConsoleFont(void) {

  // Check if the font has a PSF1 or PS2 header, and if so, return true;
  // otherwise, return false.

  uint16 PossiblePsf1Signature = *(uint16*)(ConsoleFont);
  uint32 PossiblePsf2Signature = *(uint32*)(ConsoleFont);

  if (PossiblePsf1Signature == psf1HeaderSignature) {
    return true;
  } else if (PossiblePsf2Signature == psf2HeaderSignature) {
    return true;
  }

  return false;

}


// (TODO: Something to set up the terminal table?)
