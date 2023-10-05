// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // Todo - graphics functions.
  // For now we only plan to support 80x25 text mode
  // Also itoa and all

  // ...

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

  // this isn't meant to be here it's just a placeholder to see if it works

  void PutcharAt(const char Character, uint8 Color, uint16 PosX, uint16 PosY)

  // ...

  void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void ClearTerminal(void);

  // ...

  void Scroll(void);
  void Putchar(const char Character, uint8 Color);
  void Print(const char* String, uint8 Color);

#endif
