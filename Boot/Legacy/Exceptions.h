// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EXCEPTIONS_H
#define SERRA_EXCEPTIONS_H

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
  void __attribute__((noreturn)) Panic(char* String, uint32 Eip);

  void IsrFault(uint8 Vector, uint32 Eip);
  void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error);
  void IsrAbort(uint8 Vector, uint32 Eip);
  void IsrLog(uint8 Vector);

  // This can avoid errors when including from other files.

  #include "Memory/Memory.h"
  #include "Graphics/Graphics.h"

#endif
