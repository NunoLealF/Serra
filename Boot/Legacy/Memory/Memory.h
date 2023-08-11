// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_MEMORY_H
#define SERRA_MEMORY_H

  // Todo: Implement standard memory functions; memset, memcpy, memmove.., from Memory.c

  // A20 functions, from A20/A20_C.c and A20/A20_Asm.s.

  bool CheckA20(void);
  void WaitA20(void);

  extern void EnableKbdA20(void);
  extern void EnableFastA20(void);

  // Todo: Memory map (E820 functions), from Mmap.c

  // (Todo: other functions)

#endif
