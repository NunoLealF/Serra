// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"
#include "Graphics.h"

/* void InitializeTerminal()

   Inputs: uint16 LimitX - The width (or horizontal limit) of our terminal;
           uint16 LimitY - The height (or vertical limit) of our terminal;
           uint32 Framebuffer - The memory address of the framebuffer being used.

   Outputs: (None)

   This function initializes the TerminalTable{} (type: terminalDataStruct{}) structure with the
   information given (except for PosX and PosY, which are set to zero).

   Before using any other functions (with the exception of Strlen()) in this file, one should
   always run this function at least once, in order to initialize the TerminalTable{} structure.

   Given that this is an x86 platform, and that we're *probably* on 80x25 VGA text mode, it's
   usually safe to initialize it like this:
   - InitializeTerminal(80, 25, 0xB8000)

   Also, keep in mind that this doesn't clear the terminal/framebuffer; in order to do so, you
   should use the ClearTerminal() function.

*/

void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer) {

  TerminalTable.PosX = 0;
  TerminalTable.PosY = 0;
  TerminalTable.Framebuffer = Framebuffer;

  TerminalTable.LimitX = LimitX;
  TerminalTable.LimitY = LimitY;

}


/* void ClearTerminal()

   Inputs: (None)
   Outputs: (None)

   This function essentially just clears the terminal, using the data in the TerminalTable{}
   structure (which requires InitializeTerminal() to have been run at least once).

*/

void ClearTerminal(void) {

  uint16 LimitX = TerminalTable.LimitX;
  uint16 LimitY = TerminalTable.LimitY;
  void* Framebuffer = (void*)TerminalTable.Framebuffer;

  uint32 Size = (LimitX * LimitY) * 2;

  Memset(Framebuffer, '\0', Size);

}


/* static void Scroll()

   Inputs: (None)
   Outputs: (None)

   This function 'scrolls' the terminal, using the data in the TerminalTable{} structure (which
   requires InitializeTerminal() to have been run at least once).

   Whenever the terminal gets full, this function is called in order to scroll the terminal one
   line up (discarding the first line) to make way for another line.

   Keep in mind that this function doesn't actually make any changes to the TerminalTable{}
   structure; that role belongs to UpdateTerminal(). If that function isn't accessible for
   whatever reason, do the following steps:

   - Change TerminalTable.PosX to 0;
   - Change TerminalTable.PosY to TerminalTable.LimitY - 1.

*/

static void Scroll(void) {

  // Move every line (except the first line) up, discarding the first line.

  uint32 Size = 2 * TerminalTable.LimitX * (TerminalTable.LimitY - 1);
  uint32 Offset = 2 * TerminalTable.LimitX;

  void* Destination = (void*)(TerminalTable.Framebuffer);
  void* Source = (void*)(TerminalTable.Framebuffer + Offset);

  Memmove(Destination, Source, Size);

  // Clear out the last line.

  Size = Offset;
  Destination = (void*)(TerminalTable.Framebuffer + ((TerminalTable.LimitY - 1) * Offset));

  Memset((void*)Destination, 0, Size);

}


/* static void UpdateTerminal()

   Inputs: const char Character - The character being processed.
   Outputs: (None)

   This function essentially updates the positions in the TerminalTable{} structure (which
   requires InitializeTerminal() to have been run at least once) depending on what's needed.

   It takes one input, the character being processed, so that it can emit a newline/tab/etc.
   if necessary.

   The only purpose of this function is to be called by Putchar(); after printing a given
   character, it calls this function to properly update the positions, and to move onto
   the next line / scroll / etc. if necessary.

*/

static void UpdateTerminal(const char Character) {

  // Analyze the character we were given, so we can know which changes we should make.

  switch(Character) {

    // If it's a newline ('\n'), move onto the next line, set PosX to 0, and move on.

    case '\n':

      TerminalTable.PosX = 0;
      TerminalTable.PosY++;
      break;

    // If it's a tab ('\t'), move us forward by 4 spaces/characters, and move on.

    case '\t':

      TerminalTable.PosX += 4;
      break;

    // If it's any other character, just increment PosX to move us onto the next character.

    default:
      TerminalTable.PosX++;

  }

  // Check for PosX/PosY overflowing, and move onto the next line / scroll if necessary.

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


/* static void PutcharAt()

   Inputs: const char Character - The character we want to display on the screen;
           uint8 Color - The (VGA) color we want to display on the screen.

           uint16 PosX - The horizontal (X) position to display it at;
           uint16 PosY - The vertical (Y) position to display it at.

   Outputs: (None)

   This function does one simple task: using the data in the TerminalTable{} structure, it
   displays a given character (and color) on the screen, at a certain X/Y position.

   The main function of this function is to be called by Putchar() (so it can display a character
   on the screen), but you can also call it if you want (it can be pretty useful for debugging).
   Just keep in mind that it's a static function, so it can't be called outside of Graphics.c.

*/

static void PutcharAt(const char Character, uint8 Color, uint16 PosX, uint16 PosY) {

  uint32 Offset = 2 * (PosX + (TerminalTable.LimitX * PosY));
  uint8* Framebuffer = (uint8*)(TerminalTable.Framebuffer + Offset);

  Framebuffer[0] = Character;
  Framebuffer[1] = Color;

}


/* void Putchar()

   Inputs: const char Character - The character we want to display on the screen;
           uint8 Color - The (VGA) color we want to display on the screen.

   Outputs: (None)

   This function displays a character on the screen, using the data in the TerminalTable{}
   structure (which requires InitializeTerminal() to have been run at least once), and updates
   the positions in that table and/or scrolls the terminal if necessary.

   It can be called by any function, even outside of this file (as it's not a static function),
   but its main purpose is to be called by Print(). However, if the Debug flag isn't set
   (so, if (DebugFlag == false)), this function doesn't print anything

*/

void Putchar(const char Character, uint8 Color) {

  // If Debug is false, then don't print anything (it's automatically set to true for any
  // important system messages)

  if (DebugFlag == false) {
    return;
  }

  // Depending on the character, we want to handle each of these differently:

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

    // Regular characters.

    default:

      PutcharAt(Character, Color, TerminalTable.PosX, TerminalTable.PosY);
      UpdateTerminal(Character);

  }

}


/* void Print()

   Inputs: const char* String - The string we want to display on the screen;
           uint8 Color - The (VGA) color we want to display on the screen.

   Outputs: (None)

   This function displays a string on the screen, using the data in the TerminalTable{} structure
   (which requires InitializeTerminal() to have been run at least once).

   It essentially serves the same function as Putchar(), but for strings instead of individual
   characters.

*/

void Print(const char* String, uint8 Color) {

  // If Debug is false, then don't print anything (it's automatically set to true for any
  // important system messages)

  if (DebugFlag == false) {
    return;
  }

  // Print each character in the string

  for (int i = 0; i < Strlen(String); i++) {
    Putchar(String[i], Color);
  }

}
