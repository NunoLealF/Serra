// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_BOOTLOADER_H
#define SERRA_BOOTLOADER_H

  // Import other headers

  #include "Graphics/Graphics.h"
  #include "Memory/Memory.h"
  #include "Int/Int.h"
  #include "Rm/Rm.h"

  // Declare functions in Bootloader.c

  void __attribute__((noreturn)) Init(void);
  void __attribute__((noreturn)) Bootloader(void);

  // Declare functions in Exceptions.c

  typedef enum {

    Info = 0,

    Kernel = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  void Message(messageType Type, char* String);
  char* TranslateAddress(char* Buffer, uint32 Address);
  void __attribute__((noreturn)) Panic(char* String, uint32 Eip);

  void IsrFault(uint8 Vector, uint32 Eip);
  void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error);
  void IsrAbort(uint8 Vector, uint32 Eip);
  void IsrLog(uint8 Vector);

  void IrqHandler(uint8 Vector, uint8 Port);



  // todo...

#endif
