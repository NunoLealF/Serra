// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"

// Todo - graphics functions.
/*
- [For Graphics.c]
- a function to initialize/clear the terminal
- a function to scroll through the terminal
- some sort of standard data area to store the current data (X/Y, color, etc.) in
- some sort of putchar function (that takes advantage of print)
- some sort of print function
*/

// Data area struct

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


// Initialize terminal function

void initializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer) {

  TerminalTable.PosX = 0;
  TerminalTable.PosY = 0;
  TerminalTable.Framebuffer = Framebuffer;

  TerminalTable.LimitX = LimitX;
  TerminalTable.LimitY = LimitY;

}


// Clear terminal function

void clearTerminal(void) {

  uint16 LimitX = TerminalTable.LimitX;
  uint16 LimitY = TerminalTable.LimitY;
  void* Framebuffer = (void*)TerminalTable.Framebuffer;

  uint32 Size = (LimitX * LimitY) * 2;
  
  Memset(Framebuffer, '\0', Size);

}
