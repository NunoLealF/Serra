// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers...

  #include "../Shared/Memory/Memory.h"
  #include "../Shared/Rm/Rm.h"

  #include "../Shared/Disk/Disk.h"
  #include "../Shared/InfoTable.h"

  #include "../Shared/Graphics/Graphics.h"

  // Declare functions in Bootloader.c

  [[noreturn]] void S2Init(void);
  [[noreturn]] void S2Bootloader(void);

  // Declare global variables, for use throughout the entire bootloader

  #ifdef Debug
    bool DebugFlag = Debug; // Defined by the preprocessor, use -DDebug=true or false.
  #else
    bool DebugFlag = true; // If 'Debug' isn't defined, then assume it's true
  #endif

  terminalDataStruct TerminalTable;

#endif
