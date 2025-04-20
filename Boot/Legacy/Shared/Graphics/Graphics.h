// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_SHARED_GRAPHICS_H
#define SERRA_SHARED_GRAPHICS_H

  // Exception/info message functions and data structures. (Exceptions.c)

  typedef enum {

    Info = 0,

    Boot = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  void Message(messageType Type, char* String, ...);
  void __attribute__((noreturn)) Panic(char* String, uint32 Eip);


  // Formatting and string-related functions, from Format.c

  int Strlen(const char* String);
  bool Strcmp(const char* StringA, const char* StringB);
  char* Strrev(char* String);
  char* Itoa(uint32 Number, char* Buffer, uint8 Base);

  // Type definitions, global variables and flags, from Graphics.c

  typedef volatile struct {

    // General info

    uint16 PosX;
    uint16 PosY;
    uint32 Framebuffer;

    // Mode info

    uint16 LimitX;
    uint16 LimitY;

  } __attribute__((packed)) terminalDataStruct;

  extern terminalDataStruct TerminalTable;
  extern bool Debug;


  // Functions that directly interact with the TerminalTable{} structure, from Graphics.c

  void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void ClearTerminal(void);


  // Functions that print data to the terminal, from Graphics.c

  void Putchar(const char Character, uint8 Color);
  void Print(const char* String, uint8 Color);


  // Functions that print data to the terminal, from Format.c

  void Printf(const char* String, uint8 Color, ...);
  void vPrintf(const char* String, uint8 Color, va_list Arguments);

#endif
