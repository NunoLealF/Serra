// Copyright (C) 2024 NunoLealF
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

  // Check for a specific flag in realModeTable->Eflags.

  #define hasFlag(cpuFlags, checkFlag) (cpuFlags & checkFlag)

  // Regular CPU flags - most of these are present on the 8086/286.

  #define carryFlag (1 << 0)
  #define parityFlag (1 << 2)
  #define auxCarryFlag (1 << 4)
  #define zeroFlag (1 << 6)
  #define signFlag (1 << 7)
  #define trapFlag (1 << 8)
  #define interruptFlag (1 << 9)
  #define directionFlag (1 << 10)
  #define overflowFlag (1 << 11)
  #define ioPrivilegeFlag (1 << 12)
  #define nestedFlag (1 << 14)

  // Extended CPU flags (eflags) - most of these are present on the 386/486.

  #define resumeFlag (1 << 16)
  #define virtual86Flag (1 << 17)
  #define alignFlag (1 << 18)
  #define virtualInterruptFlag (1 << 19)
  #define virtualInterruptPendingFlag (1 << 20)
  #define cpuidFlag (1 << 21)

  // Real mode functions.

  void RealMode(void);
  realModeTable* InitializeRealModeTable(void);

#endif
