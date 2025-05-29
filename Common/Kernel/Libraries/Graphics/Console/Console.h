// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_GRAPHICS_CONSOLE_H
#define SERRA_KERNEL_GRAPHICS_CONSOLE_H

  // Include standard headers.

  #include "../../Stdint.h"

  // Include console-related definitions

  typedef struct _consoleSubsystemData {

    // (Mode and positioning information)

    bool IsSupported; // (Does this system support a graphics mode?)

    enum : uint8 {

      UnknownConsole = 0, // (Not defined)
      GraphicalConsole = 1, // (Uses the graphics subsystem to display text)
      VgaConsole = 2, // (Uses the VGA text buffer at B8000h)
      EfiConsole = 3 // (Uses efiSimpleTextOutputProtocol)

    } Type;

    uint16 PosX; // (The current horizontal/X position, in characters - optional for EfiConsole)
    uint16 PosY; // (The current vertical/Y position, in characters - optional for EfiConsole)

    uint16 LimitX; // (The horizontal/X resolution, in characters)
    uint16 LimitY; // (The vertical/Y resolution, in characters)

  } consoleSubsystemData;

  // Include functions and global variables from Console.c

  extern consoleSubsystemData ConsoleData;
  extern bool ConsoleEnabled;
  extern bool DebugFlag;

  bool InitializeConsoleSubsystem(void* InfoTable);

  void Putchar(const char Character, bool Important, uint8 Attribute);
  void Print(const char* String, bool Important, uint8 Attribute);

  // Include platform-specific functions from Efi.c

  void Putchar_Efi(const char Character, int32 Attribute);
  void Print_Efi(const char* String, int32 Attribute);

  // Include message functions and data structures from Exceptions.c

  typedef enum _messageType : int8 {

    Info = 0,

    Kernel = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  void Message(messageType Type, const char* String, ...);

  // Include formatting-related functions from Format.c

  void Printf(const char* String, bool Important, uint8 Color, ...);
  void vPrintf(const char* String, bool Important, uint8 Color, va_list Arguments);

  // Include platform-specific functions from Graphical.c

  void Putchar_Graphical(const char Character, uint8 Attribute);
  void Print_Graphical(const char* String, uint8 Attribute);

  // Includeplatform-specific  functions from Vga.c

  void Putchar_Vga(const char Character, uint8 Attribute);
  void Print_Vga(const char* String, uint8 Attribute);

#endif
