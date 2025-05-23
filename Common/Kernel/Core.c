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

      uint64 RedMask = ((InfoTable->Display.Graphics.Bits.RedMask >> 3) & InfoTable->Display.Graphics.Bits.RedMask);
      uint64 GreenMask = ((InfoTable->Display.Graphics.Bits.GreenMask >> 1) & InfoTable->Display.Graphics.Bits.GreenMask);
      uint64 BlueMask = ((InfoTable->Display.Graphics.Bits.BlueMask >> 0) & InfoTable->Display.Graphics.Bits.BlueMask);

      auto RedI = (Y * RedMask / InfoTable->Display.Graphics.LimitY) & InfoTable->Display.Graphics.Bits.RedMask;
      auto GreenI = (Y * GreenMask / InfoTable->Display.Graphics.LimitY) & InfoTable->Display.Graphics.Bits.GreenMask;
      auto BlueI = (Y * BlueMask / InfoTable->Display.Graphics.LimitY) & InfoTable->Display.Graphics.Bits.BlueMask;

      auto Intensity = 0;

      // (This shows the CPU features)

      if (CpuFeaturesAvailable.Avx512f == true) {

        Intensity |= InfoTable->Display.Graphics.Bits.RedMask;
        Intensity |= InfoTable->Display.Graphics.Bits.GreenMask;
        Intensity |= InfoTable->Display.Graphics.Bits.BlueMask;

        Intensity /= InfoTable->Display.Graphics.LimitY;
        Intensity *= Y;

      } else if (Y % 2 == 0) {

        if (CpuFeaturesAvailable.Sse2 == true) Intensity |= RedI;
        if (CpuFeaturesAvailable.Ssse3 == true) Intensity |= GreenI;
        if (CpuFeaturesAvailable.Avx == true) Intensity |= BlueI;

      } else {

        if (CpuFeaturesAvailable.Sse3 == true) Intensity |= RedI;
        if (CpuFeaturesAvailable.Sse4 == true) Intensity |= GreenI;
        if (CpuFeaturesAvailable.Avx2 == true) Intensity |= BlueI;

      }

      if (InfoTable->Display.Graphics.Bits.PerPixel <= 16) {
        Memset((void*)Buffer, (uint16)Intensity, Size);
      } else if (InfoTable->Display.Graphics.Bits.PerPixel <= 32) {
        Memset((void*)Buffer, (uint32)Intensity, Size);
      } else if (InfoTable->Display.Graphics.Bits.PerPixel <= 64) {
        Memset((void*)Buffer, (uint64)Intensity, Size);
      }

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
