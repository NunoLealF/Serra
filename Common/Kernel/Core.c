// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    for (auto Y = 0; Y < (InfoTable->Display.Graphics.LimitY-2); Y++) {

      // (Get necessary addresses and values)

      auto Buffer = (InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * Y));
      auto Size = (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 8);

      // (Calculate the necessary value, and try to fill this line)

      uint8 Intensity = (Y * 255 / InfoTable->Display.Graphics.LimitY);
      Memset((void*)Buffer, Intensity, Size);

    }

  }

  // (Depending on the system type, either wait for a keypress or just
  // stall the system for a while)

  if ((InfoTable->Firmware.Type == EfiFirmware) && (InfoTable->Firmware.Efi.SupportsConIn == true)) {

    // (Wait for a keypress)

    efiInputKey PhantomKey;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

  } else {

    // (Do a lot of NOPs to stall the system)

    for (auto Count = 0; Count < 500000000; Count++) {
      __asm__ __volatile__ ("nop");
    }

  }

  // (Return.)

  return;

}
