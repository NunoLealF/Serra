// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "../../../Constructors/Firmware/Firmware.h"
#include "Console.h"

// (TODO - Add a function to (quickly) convert between ASCII and UTF-16;
// maybe you could use SSE's interleave/interpack/whatever functions?)



// (TODO - Add functions to convert from a RGB background/foreground to
// the most appropriate(?) VGA value)

static inline uint8 ConvertToEfiAttribute(uint32 BackgroundColor, uint32 ForegroundColor) [[reproducible]] {

  return 0;

}



// (TODO - Add functions for interfacing with gST->ConOut)

void PrintEfi(const char* String, uint32 BackgroundColor, uint32 ForegroundColor) {

  // (Convert the background and foreground colors to the 'closest'
  // efiSimpleTextOutputProtocol color attribute)

  uint8 Attribute = ConvertToEfiAttribute(BackgroundColor, ForegroundColor);

  // (TODO TODO TODO TODO / Display something / TODO TODO TODO TODO TODO TODO)

  // (Return.)

  return;

}
