// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

// The code that actually does this is at C000h and is compiled by NASM; this is just a wrapper
// lol.

void realMode(void) {

  __asm__("movl $0xC000, %%eax; call *%%eax" : : : "eax");

}
