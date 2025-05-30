// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_SYSTEM_H
#define SERRA_KERNEL_SYSTEM_H

  // Include the relevant header file for our architecture.

  #if defined(__amd64__) || defined(__x86_64__)

    #define SystemPageSize 4096
    #include "x64/x64.h"

  #elif defined(__aarch64__)

    #define SystemPageSize 16384 // (I've heard M1 Macs do this)
    #error "ARM64 targets are not supported :("

  #elif defined(__riscv) || defined(__riscv__)

    #error "RISC-V targets are not supported :("

  #else

    #error "Could not detect the system architecture."

  #endif

#endif
