// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

// The code that actually does this is at C000h and is compiled by NASM; this is just a wrapper
// lol.

void realMode(void) {

  __asm__("movl $0xC000, %%eax; call *%%eax" : : : "eax");

}

// C000-C800: Protected -> real mode code
// C800-D000: Real -> protected mode code
// D000-DE00: Real mode payload
// DE00-E000: Data

// (This is supposed to be stored and then later on read by the real-mode code btw.)
// (Probably should allocate it at like, DF00 tbh)
// You want a function that 'initializes' this I think, DF00 would be the best area imo.

// Right now, this is 36 bytes long
// Also, it's a typedef, so you have to declare like a new struct at DF00

typedef struct {

  uint32 Eax;
  uint32 Ebx;
  uint32 Ecx;
  uint32 Edx;

  uint32 Esi;
  uint32 Edi;
  uint32 Ebp;

  uint16 Ds;
  uint16 Es;
  uint16 Fs;
  uint16 Gs;

} __attribute__((packed)) realModeRegisters;
