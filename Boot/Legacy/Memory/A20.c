// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

// To-do: Make functions related to A20.

// You probably want to use the Fast A20 method (supported on most PCs since PS/2)
// But first, check if it's already enabled (most modern PCs enable it by default)

bool CheckA20(void) {

  // On some systems, the A20 line isn't enabled by default.
  // (I don't know why but I just can't write documentation right now, sorry)
  // (We'll test at FC00h)

  uint32 magicNumber = 0x31323665;

  // ............

  volatile uint32* testLocation = (volatile uint32*)0xFC00;

  *(uint32*)0xFC00 = magicNumber;
  *(uint32*)0x10FC00 = ~magicNumber;

  // ............

  if (*testLocation == magicNumber) {

    return true;

  } else {

    return false;

  }

}
