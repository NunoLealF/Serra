// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    for (auto Y = 0; Y < (InfoTable->Display.Graphics.LimitY); Y++) {

      // (Get necessary addresses and values)

      auto Buffer = (InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * Y));
      auto Size = (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 8);

      // (Use memset to display a pretty gradient)

      Memset((void*)Buffer, (uint8)(Y * 255 / InfoTable->Display.Graphics.LimitY), Size);

    }

    // (Draw a bezier curve)

    uint16 Width = 480;
    uint16 Height = 480;

    uint16 WidthOffset = (InfoTable->Display.Graphics.LimitX - Width) / 2;
    uint16 HeightOffset = (InfoTable->Display.Graphics.LimitY - Height) / 2;

    double Points[3][2] = {{0, 0.5}, {0, 0}, {0.5, 0}};

    for (auto j = 0; j < 1024; j++) {

      uint16 Bezier[2];

      double a = j / 1024.f;
      double b = (1024-j) / 1024.f;

      Bezier[0] = (uint16)((double)Width * ((a * a * Points[0][0]) + (2 * a * b * Points[1][0]) + (b * b * Points[2][0]))) + WidthOffset;
      Bezier[1] = (uint16)((double)Height * ((a * a * Points[0][1]) + (2 * a * b * Points[1][1]) + (b * b * Points[2][1]))) + HeightOffset;

      uint64 Addr = InfoTable->Display.Graphics.Framebuffer.Address;
      Addr += (Bezier[1] * InfoTable->Display.Graphics.Pitch);
      Addr += (Bezier[0] * InfoTable->Display.Graphics.Bits.PerPixel / 8);

      Memset((void*)Addr, 0xFF, (InfoTable->Display.Graphics.Bits.PerPixel / 8));

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
