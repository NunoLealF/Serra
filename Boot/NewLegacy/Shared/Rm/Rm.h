// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_REALMODE_H
#define SERRA_REALMODE_H

  // Real mode register table - initialize at (9E00h + E00h) = AC00h.
  // See more info in RmWrapper.c

  typedef struct {

    // General purpose registers.

    volatile uint32 Eax;
    volatile uint32 Ebx;
    volatile uint32 Ecx;
    volatile uint32 Edx;

    // Segment registers.

    volatile uint16 Ds;
    volatile uint16 Es;

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

  #define hasFlag(cpuFlags, checkFlag) ((cpuFlags & checkFlag) != 0)

  // Regular CPU flags - most of these are present on the 8086/286.

  #define CarryFlag (1 << 0)
  #define ParityFlag (1 << 2)
  #define AuxCarryFlag (1 << 4)
  #define ZeroFlag (1 << 6)
  #define SignFlag (1 << 7)
  #define TrapFlag (1 << 8)
  #define InterruptFlag (1 << 9)
  #define DirectionFlag (1 << 10)
  #define OverflowFlag (1 << 11)
  #define IoPrivilegeFlag (1 << 12)
  #define NestedFlag (1 << 14)

  // Extended CPU flags (eflags) - most of these are present on the 386/486.

  #define ResumeFlag (1 << 16)
  #define Virtual86Flag (1 << 17)
  #define AlignFlag (1 << 18)
  #define VirtualInterruptFlag (1 << 19)
  #define VirtualInterruptPendingFlag (1 << 20)
  #define CpuidFlag (1 << 21)

  // Real mode functions.

  void RealMode(void);
  realModeTable* InitializeRealModeTable(void);

#endif
