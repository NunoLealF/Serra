// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EFI_GRAPHICS_H
#define SERRA_EFI_GRAPHICS_H

  // Exception/info message functions and data structures, from Exceptions.c.

  typedef enum _messageType : int8 {

    Info = 0,

    Boot = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  void Message(messageType Type, char16* String, ...);


  // Global variables, from Bootloader.h

  #ifndef SERRA_BOOTLOADER_H

    extern bool DebugFlag;
    extern bool SupportsConOut;

  #endif


  // Formatting and string-related functions, from Format.c.

  int Strlen(const char16* String);
  bool Strcmp(const char16* StringA, const char16* StringB);
  bool StrcmpShort(const char8* StringA, const char8* StringB);
  char16* Strrev(char16* String);
  char16* Itoa(uint64 Number, char16* Buffer, uint8 Base);


  // Functions that interact with or print to the console, from Graphics.c

  void ClearTerminal(void);
  void Putchar(const char16 Character, int32 Color);
  void Print(const char16* String, int32 Color);


  // Functions that print to the console, from Format.c

  void Printf(const char16* String, int32 Color, ...);
  void vPrintf(const char16* String, int32 Color, va_list Arguments);

#endif
