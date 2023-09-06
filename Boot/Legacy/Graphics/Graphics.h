// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // Todo - graphics functions.
  // For now we only plan to support 80x25 text mode
  // Also itoa and all

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

  void initializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void clearTerminal(void);

#endif
