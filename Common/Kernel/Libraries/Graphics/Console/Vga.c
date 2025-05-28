// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Stdint.h"
#include "Console.h"

// (TODO - Add a function to scroll the framebuffer, that hopefully isn't
// extremely slow (hopefully my optimized Memcpy shines here))



// (TODO - Add functions to convert from a RGB background/foreground to
// the most appropriate(?) VGA value)

static inline uint8 ConvertToVgaAttribute(uint32 BackgroundColor, uint32 ForegroundColor) [[reproducible]] {

  return 0;

}



// (TODO - Add a simple Putchar/Print function, as in the BIOS loader)

void PrintVga(const char* String, uint32 BackgroundColor, uint32 ForegroundColor) {

  // (Convert the background and foreground colors to the 'closest'
  // VGA attribute)

  uint8 Attribute = ConvertToVgaAttribute(BackgroundColor, ForegroundColor);

  // (TODO TODO TODO TODO / Display something / TODO TODO TODO TODO TODO TODO)

  // (Return.)

  return;

}
