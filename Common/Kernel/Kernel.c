// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Kernel.h"

// TODO - Write documentation

void Kernel(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    for (auto Y = 0; Y < InfoTable->Display.Graphics.LimitY; Y++) {

      // (Get necessary addresses and values)

      auto Buffer = (InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * Y));
      auto Size = (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 8);
      uint8 Intensity8 = (Y * 255 / InfoTable->Display.Graphics.LimitY);

      // (Use SSE to display a gradient, 16 bytes at a time why not)

      while ((Buffer % 16) != 0) {
        *(uint8*)(Buffer) = Intensity8;
        Buffer++; Size--;
      }

      alignas(16) uint16 Intensity16 = ((uint16)Intensity8 << 8) | Intensity8;

      __asm__ __volatile__ (".intel_syntax noprefix;"
                            "pshuflw xmm0, [%0], 0;"
                            "punpcklwd xmm0, xmm0;"
                            "_SseLoop:"
                            "movntdq [%1], xmm0;"
                            "add %1, 16; dec %2;"
                            "cmp %2, 0;"
                            "jne _SseLoop;"
                            ".att_syntax prefix" :: "r"(&Intensity16), "r"(Buffer), "r"(Size) : "memory", "xmm0");

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
