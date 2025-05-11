// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EFI_CPU_H
#define SERRA_EFI_CPU_H

  // (Functions and definitions, from Cpu.c)

  void WriteToMsr(uint64 Msr, volatile uint64 Value);
  uint64 ReadFromMsr(uint64 Msr);

  void WriteToControlRegister(uint8 Register, volatile uint64 Value);
  uint64 ReadFromControlRegister(uint8 Register);

  uint8 GetCpuProtectionLevel(void);

  // (Macros, for use with bitwise operations)

  #define clearBit(Value, Bit) (Value & (typeof(Value))(~(1ULL << Bit)))
  #define setBit(Value, Bit) (Value | (typeof(Value))((1ULL << Bit)))

#endif
