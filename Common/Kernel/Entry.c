// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "../InfoTable.h"

#if !defined(__amd64__) || !defined(__x86_64__)
  #error "This code must be compiled with an x86_64-elf cross-compiler"
#endif

void Entrypoint(uintptr InfoTablePtr) {

  // Aaaaaa

  kernelInfoTable* InfoTable = (kernelInfoTable*)InfoTablePtr;

  // Let's test this out

  /*

  uint32* Vesathing = (uint32*)(InfoTable->Graphics.Vesa.Framebuffer);

  for (int i = 0; i < (640*480); i++) {
    Vesathing[i] = i;
  }

  */

  uintptr ThingAddr = (uintptr)InfoTable->Graphics.VgaText.Framebuffer;

  uint16* Thing = (uint16*)ThingAddr;
  char* Thing2 = "Hi, this is kernel mode Serra! <3";
  char* Thing3 = "April 25 2025";

  int Position = 0;

  for (int a = 0; a < (80*3); a++) {
    Thing[a] = 0;
  }

  while (Thing2[Position] != '\0') {
    Thing[Position] = 0x0F00 + Thing2[Position];
    Position++;
  }

  Position = 0;

  while (Thing3[Position] != '\0') {
    Thing[Position + 80] = 0x3F00 + Thing3[Position];
    Position++;
  }

  // TODO: Set up environment

  // (0) Make it so that the stub can eventually return.
  // (1) Check for problems - is SSE not enabled, sanity-check things, etc.
  // (2) Set up basic IDT, panic handling, etc.
  // (3) Actually

  for(;;);

  // (Return?)

  return;

}
