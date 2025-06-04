// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../../Common.h"
#include "../../Firmware/Firmware.h"
#include "../Graphics.h"
#include "Console.h"

// (TODO - Global variables)

consoleSubsystemData ConsoleData = {0};
bool ConsoleEnabled = false;

#ifdef Debug
  bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
#else
  bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
#endif



// (TODO - Some sort of common.. console subsystem, maybe? I just need a
// printf that will work nearly universally, I guess)

// (This depends on the graphics subsystem for actually doing anything,
// and on the commonInfoTable for initializing it; keep in mind that widths
// must be padded for 24-bit and 48-bit systems)

// (Which is to say... if you have an odd resolution, just assume it to be
// even)

bool InitializeConsoleSubsystem(void* InfoTable) {

  // Before we do anything else, let's check that the pointer we were
  // given actually matches a commonInfoTable{} structure.

  commonInfoTable* Table = (commonInfoTable*)InfoTable;

  if (Table->Signature != commonInfoTableSignature) {
    return false;
  }

  // Next, let's check to see whether we're in a text or a graphics mode.

  if (GraphicsData.IsSupported == true) {

    // (If we're here, then the graphics subsystem has been initialized,
    // which means we're in a graphics mode, so let's deal with that)

    ConsoleData.IsSupported = true;
    ConsoleData.Type = GraphicalConsole;

  } else {

    // (If we're here, then we either don't have any graphics output, or
    // we're in a text mode of some sort, so let's deal with that)

    switch (Table->Display.Type) {

      case UnknownDisplay:
        ConsoleData.IsSupported = false;
        break;

      case VgaDisplay:
        ConsoleData.IsSupported = true;
        ConsoleData.Type = VgaConsole;
        break;

      case EfiTextDisplay:
        ConsoleData.IsSupported = true;
        ConsoleData.Type = EfiConsole;
        break;

      default:
        return false;

    }

  }

  // Now that we know which mode we're in, let's initialize
  // ConsoleData{} with the correct information:

  if (ConsoleData.Type == GraphicalConsole) {

    // If we're in a graphics mode, that means we need to take care of
    // displaying text ourselves, which means we need to use a bitmap
    // font, and calculate our limits ourselves.

    // (Check to see if a bitmap font has been initialized; if not,
    // try to initialize it ourselves, and if that fails, return)

    if (BitmapFontData.IsSupported == false) {

      if (InitializeBitmapFont() == false) {
        return false;
      } else if (BitmapFontData.IsSupported == false) {
        return false;
      }

    }

    // (Set the starting position to (0, 0))

    ConsoleData.PosX = 0;
    ConsoleData.PosY = 0;

    // (Calculate limits, by dividing the (even) screen resolution
    // by the font resolution)

    ConsoleData.LimitX = (GraphicsData.LimitX & 0xFFFE) / BitmapFontData.Width;
    ConsoleData.LimitY = (GraphicsData.LimitY & 0xFFFE) / BitmapFontData.Height;

  } else if (ConsoleData.Type == VgaConsole) {

    // If we're in a VGA text mode, then that means we have a hardware-
    // -supported framebuffer that displays characters for us, with a
    // defined limit, so we just need to keep track of the position.

    // (Set the starting position to what's specified by commonInfoTable{})

    ConsoleData.PosX = Table->Display.Text.PosX;
    ConsoleData.PosY = Table->Display.Text.PosY;

    // (Set the limits to what's specified by commonInfoTable{})

    ConsoleData.LimitX = Table->Display.Text.LimitX;
    ConsoleData.LimitY = Table->Display.Text.LimitY;

  } else if (ConsoleData.Type == EfiConsole) {

    // If we're in an EFI text mode, then that means we have a hardware-
    // -supported framebuffer that displays characters for us, with a well-
    // -defined limit, and with no need to keep track of the position.

    // (Check that EFI tables have been initialized; if not, try to
    /// initialize it ourselves, and if that fails, return)

    if (gST == NULL) {

      InitializeEfiTables(Table->Firmware.Efi.SystemTable.Pointer);

      if (gST != Table->Firmware.Efi.SystemTable.Pointer) {
        return false;
      }

    }

    // (Set the limits to what's specified by commonInfoTable{})

    ConsoleData.LimitX = Table->Display.Text.LimitX;
    ConsoleData.LimitY = Table->Display.Text.LimitY;

  }

  // (Enable the console (if applicable), and return true.)

  ConsoleEnabled = ConsoleData.IsSupported;
  return true;

}



// (TODO: Like Print(), but for characters)

void Putchar(const char Character, bool Important, uint8 Attribute) {

  // (If the console is disabled, or both the Debug flag and `Important`
  // is false, return)

  if (ConsoleEnabled == false) {
    return;
  } else if ((DebugFlag == false) && (Important == false)) {
    return;
  }

  // (Depending on the console type, pick the right function to print with)

  if (ConsoleData.Type == EfiConsole) {
    Putchar_Efi(Character, (int32)Attribute);
  } else if (ConsoleData.Type == GraphicalConsole) {
    Putchar_Graphical(Character, Attribute);
  } else if (ConsoleData.Type == VgaConsole) {
    Putchar_Vga(Character, Attribute);
  }

  // (Return.)

  return;

}



// (TODO: Something to print a string on the screen - can be VGA/EFI/graphical)

void Print(const char* String, bool Important, uint8 Attribute) {

  // (If the console is disabled, or both the Debug flag and `Important`
  // is false, return)

  if (ConsoleEnabled == false) {
    return;
  } else if ((DebugFlag == false) && (Important == false)) {
    return;
  }

  // (Depending on the console type, pick the right function to print with)

  if (ConsoleData.Type == EfiConsole) {
    Print_Efi(String, (int32)Attribute);
  } else if (ConsoleData.Type == GraphicalConsole) {
    Print_Graphical(String, Attribute);
  } else if (ConsoleData.Type == VgaConsole) {
    Print_Vga(String, Attribute);
  }

  // (Return.)

  return;

}
