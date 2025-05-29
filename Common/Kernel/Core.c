// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if (GraphicsData.IsSupported == true) {

    for (auto Y = 0; Y < GraphicsData.LimitY; Y++) {

      // (Get necessary addresses and values)

      auto Buffer = ((uintptr)GraphicsData.Framebuffer + (GraphicsData.Pitch * Y));
      auto Size = (GraphicsData.LimitX * GraphicsData.Bpp);

      // (Use memset to display a pretty gradient)

      Memset((void*)Buffer, (uint8)(Y * 256 / GraphicsData.LimitY), Size);

    }

  }

  // (Test Print)

  Print("Hi, this is graphics-mode Serra! <3\n\r", true, 0x0F);

  for (uint8 Attribute = 0; Attribute < 0x0F; Attribute++) {
    Print("May 29 2025", true, 0x3F);
    Putchar(' ', true, 0);
    Print("[Attribute]\n\r", true, Attribute);
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
