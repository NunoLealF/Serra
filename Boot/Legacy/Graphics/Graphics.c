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

  // Move

  uint32 Size = 2 * TerminalTable.LimitX * (TerminalTable.LimitY - 1);
  uint32 Offset = 2 * TerminalTable.LimitX;

  void* Destination = (void*)(TerminalTable.Framebuffer);
  void* Source = (void*)(TerminalTable.Framebuffer + Offset);

  Memmove(Destination, Source, Size);

  // Clear the last line as well

  Size = Offset;
  Destination = (void*)(TerminalTable.Framebuffer + (24 * Offset));

  Memset((void*)Destination, 0, Size);

}


// UpdateTerminal function

static void UpdateTerminal(const char Character) {

  // Character checks

  switch(Character) {

    // Newline.

    case '\n':

      TerminalTable.PosX = 0;
      TerminalTable.PosY++;
      break;

    // Tab.

    case '\t':

      TerminalTable.PosX += 4;
      break;

    // Any other regular character.

    default:
      TerminalTable.PosX++;

  }

  // Check for overflows.

  if (TerminalTable.PosX >= TerminalTable.LimitX) {

    TerminalTable.PosX = 0;
    TerminalTable.PosY++;

  }

  if (TerminalTable.PosY >= TerminalTable.LimitY) {

    TerminalTable.PosX = 0;
    TerminalTable.PosY = (TerminalTable.LimitY - 1);

    Scroll();

  }

}


// PutcharAt function

static void PutcharAt(const char Character, uint8 Color, uint16 PosX, uint16 PosY) {

  uint32 Offset = 2 * (PosX + (TerminalTable.LimitX * PosY));
  uint8* Framebuffer = (TerminalTable.Framebuffer + Offset);

  Framebuffer[0] = Character;
  Framebuffer[1] = Color;

}


// Putchar function

void Putchar(const char Character, uint8 Color) {

  switch (Character) {

    // Null bytes and other escape sequences (these will pretty much be ignored)

    case '\0':
      break;

    case '\b':
      break;

    case '\f':
      break;

    case '\r':
      break;

    case '\v':
      break;

    // Newlines

    case '\n':

      UpdateTerminal(Character);
      break;

    // Tabs.

    case '\t':

      UpdateTerminal(Character);
      break;

    // Regular character.

    default:

      PutcharAt(Character, Color, TerminalTable.PosX, TerminalTable.PosY);
      UpdateTerminal(Character);

  }

}


// Print function

void Print(const char* String, uint8 Color) {

  for (unsigned int i = 0; i < Strlen(String); i++) {
    Putchar(String[i], Color);
  }

}
