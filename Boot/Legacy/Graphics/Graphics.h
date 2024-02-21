// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_GRAPHICS_H
#define SERRA_GRAPHICS_H

  // Formatting and string-related functions. (Format.c)

  int Strlen(const char* String);
  char* Strrev(char* String);
  char* Itoa(uint32 Number, char* Buffer, uint8 Base);

  // The terminalDataStruct type, and TerminalTable{}. (Graphics.c)

  typedef volatile struct {

    // General info

    uint16 PosX;
    uint16 PosY;
    uint32 Framebuffer;

    // Mode info

    uint16 LimitX;
    uint16 LimitY;

  } terminalDataStruct;

  extern terminalDataStruct TerminalTable;

  // Functions that directly interact with the TerminalTable{} structure. (Graphics.c)

  void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void ClearTerminal(void);

  // Functions that print data to the terminal. (Graphics.c)

  void Putchar(const char Character, uint8 Color);
  void Print(const char* String, uint8 Color);

  // Functions that print data to the terminal. (Format.c)

  void Printf(const char* String, uint8 Color, ...);

#endif
