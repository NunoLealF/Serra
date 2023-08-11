// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_RM_H_
#define SERRA_RM_H

  // Real mode register table - initialize at CE00.
  // See more info in RmWrapper.c

  typedef struct {

    // General purpose registers.

    volatile uint32 Eax;
    volatile uint32 Ebx;
    volatile uint32 Ecx;
    volatile uint32 Edx;

    // Increment/decrement registers.

    volatile uint16 Si;
    volatile uint16 Di;

    // Base pointer register.

    volatile uint16 Bp;

    // Eflags register (output-only).

    const volatile uint32 Eflags;

    // Interrupt that should be called (input-only).

    uint8 Int;

  } __attribute__((packed)) realModeTable;

  // Real mode functions.

  void realMode(void);
  realModeTable* initializeRealModeTable(void);

#endif
