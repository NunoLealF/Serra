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

void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer) {

  TerminalTable.PosX = 0;
  TerminalTable.PosY = 0;
  TerminalTable.Framebuffer = Framebuffer;

  TerminalTable.LimitX = LimitX;
  TerminalTable.LimitY = LimitY;

}


// Clear terminal function

void ClearTerminal(void) {

  uint16 LimitX = TerminalTable.LimitX;
  uint16 LimitY = TerminalTable.LimitY;
  void* Framebuffer = (void*)TerminalTable.Framebuffer;

  uint32 Size = (LimitX * LimitY) * 2;

  Memset(Framebuffer, '\0', Size);

}


// Scroll function

void Scroll(void) {

  uint32 Size = 2 * TerminalTable.LimitX * (TerminalTable.LimitY - 1);
  uint32 Offset = 2 * TerminalTable.LimitX;

  void* Destination = (void*)(TerminalTable.Framebuffer);
  void* Source = (void*)(TerminalTable.Framebuffer + Offset);

  Memmove(Destination, Source, Size);

}


// PutcharAt function

void PutcharAt(const char Character, uint8 Color, uint16 PosX, uint16 PosY) {

  uint32 Offset = 2 * (PosX + (TerminalTable.LimitX * PosY));
  uint8* Framebuffer = (TerminalTable.Framebuffer + Offset);

  Framebuffer[0] = Character;
  Framebuffer[1] = Color;

}


// Putchar function

void Putchar(const char Character, uint8 Color) {

  switch (Character) {

    case '\0':
      // ...
      break;

    case '\n':
      // ...
      break;

    case '\t':
      // ...
      break;

    default:
      // ...

  }

}


// Print function

void Print(const char* String, uint8 Color) {

  for (unsigned int i = 0; i < Strlen(String); i++) {
    Putchar(String[i], Color);
  }

}
