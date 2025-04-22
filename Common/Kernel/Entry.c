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

  KernelInfoTable* InfoTable = (KernelInfoTable*)InfoTablePtr;

  // Let's test this out

  unsigned short* Thing = (unsigned short*)(InfoTable->Graphics.Vga.Framebuffer.Address);
  char* Thing2 = "Hi, this is kernel mode Serra! <3";
  char* Thing3 = "April 22 2025";

  int Position = 0;

  for (int a = 0; a < (80*2); a++) {
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
  // TODO: Set up basic IDT, panic handling, etc.

  for(;;);

  // (Return?)

  return;

}
