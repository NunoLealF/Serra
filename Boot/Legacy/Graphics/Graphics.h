// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // The terminalDataStruct type, and TerminalTable{}.

  typedef volatile struct {

    // General info

    uint16 PosX;
    uint16 PosY;
    uint32 Framebuffer;

    // Mode info

    uint16 LimitX;
    uint16 LimitY;

  } terminalDataStruct;

  terminalDataStruct TerminalTable;

  // Functions that directly interact with the TerminalTable{} structure.

  void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void ClearTerminal(void);

  // Functions that print data to the terminal.

  void Putchar(const char Character, uint8 Color);
  void Print(const char* String, uint8 Color);

#endif
