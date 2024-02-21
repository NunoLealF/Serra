// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EXCEPTIONS_H
#define SERRA_EXCEPTIONS_H

  // Data structures in Exceptions.c.

  typedef enum {

    Info = 0,

    Kernel = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  // Functions in Exceptions.c.

  void Message(messageType Type, char* String);
  void __attribute__((noreturn)) Panic(char* String, uint32 Eip);

#endif
