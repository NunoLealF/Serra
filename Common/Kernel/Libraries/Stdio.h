// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_STDIO_H
#define SERRA_KERNEL_STDIO_H

  // Include standard headers.

  #include "Stdint.h"

  // Include input-related functions from ...TODO

  // Include output-related functions from Graphics/Console/Console.c,
  // Exceptions.c and Format.c.

  #ifndef SERRA_KERNEL_GRAPHICS_CONSOLE_H

    void Putchar(const char Character, bool Important, uint8 Attribute);
    void Print(const char* String, bool Important, uint8 Attribute);

    void vPrintf(const char* String, bool Important, uint8 Color, va_list Arguments);
    void Printf(const char* String, bool Important, uint8 Color, ...);

    typedef enum _messageType : int8 {

      Info = 0,

      Kernel = 1,
      Ok = 2,
      Fail = 3,
      Warning = 4,

      Error = 5

    } messageType;

    void Message(messageType Type, const char* String, ...);

  #endif

#endif
