// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

// The code that actually does this is at C000h and is compiled by NASM; this is just a wrapper
// lol.

void realMode(void) {

  __asm__("movl $0xC000, %%eax; call *%%eax" : : : "eax");

}

// (This is supposed to be stored and then later on read by the real-mode code btw.)
// (Allocate at CE00)

// Right now, this is 22 bytes long
// Also, it's a typedef, so you have to declare like a new struct at CE00

// This is input and output, so we can read eflags later

typedef struct {

  // General purpose registers

  uint32 Eax;
  uint32 Ebx;
  uint32 Ecx;
  uint32 Edx;

  // Increment/decrement registers

  uint16 Si;
  uint16 Di;
  uint16 Bp;

  // Eflags (output only)

  uint32 Eflags;

} __attribute__((packed)) realModeRegisters;

// Function

void loadRegisters(uint32 Eax, uint32 Ebx, uint32 Ecx, uint32 Edx, uint16 Si, uint16 Di, uint16 Bp) {

  realModeRegisters* Table = (void*)0xCE00;

  Table->Eax = Eax;
  Table->Ebx = Ebx;
  Table->Ecx = Ecx;
  Table->Edx = Edx;

  Table->Si = Si;
  Table->Di = Di;
  Table->Bp = Bp;

}
