// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_GRAPHICS_H
#define SERRA_GRAPHICS_H

  // Exception/info message functions and data structures. (Exceptions.c)

  typedef enum {

    Info = 0,

    Kernel = 1,
    Ok = 2,
    Fail = 3,
    Warning = 4,

    Error = 5

  } messageType;

  void Message(messageType Type, char* String, ...);
  void __attribute__((noreturn)) Panic(char* String, uint32 Eip);


  // Formatting and string-related functions. (Format.c)

  int Strlen(const char* String);
  char* Strrev(char* String);
  char* Itoa(uint32 Number, char* Buffer, uint8 Base);


  // The terminalDataStruct type, TerminalTable{}, and the debug (show non-important
  // messages) flag. (Graphics.c)

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
  extern volatile bool Debug;


  // Functions that directly interact with the TerminalTable{} structure. (Graphics.c)

  void InitializeTerminal(uint16 LimitX, uint16 LimitY, uint32 Framebuffer);
  void ClearTerminal(void);


  // Functions that print data to the terminal. (Graphics.c)

  void Putchar(const char Character, uint8 Color);
  void Print(const char* String, uint8 Color);


  // Functions that print data to the terminal. (Format.c)

  void Printf(const char* String, uint8 Color, ...);
  void vPrintf(const char* String, uint8 Color, va_list Arguments);


  // (TODO: structures from Vbe.c)

  typedef struct {

    // (VBE 1.x)

    uint32 Signature; // This should just be 'VESA', or 'VBE2'? I'm not sure
    uint16 Version; // Usually the high 8 bits are the overall version number, so, for example, VBE 1.1 is 1010h, and VBE 3.0 is 3000h

    farPtr OemStringPtr; // Far pointer to a null-terminated string
    uint32 Capabilities; // This is a bitfield (defined in the VESA spec)
    farPtr VideoModeListPtr; // Far pointer to a list of 16-byte mode numbers, where the last entry is FFFFh (to mark the end of the list)
    uint16 NumBlocks; // The number of memory blocks (64 KiB each)

    // (VBE 2.x and higher)

    struct __OemInfo {

      uint16 VbeRevision;
      farPtr VendorNamePtr;
      farPtr ProductNamePtr;
      farPtr ProductRevPtr;

    } __attribute__((packed)) OemInfo;

    // (Reserved for future use)

    uint8 Reserved[222]; // Reserved for future VESA/VBE versions
    uint8 OemData[256]; // Reserved by the OEM; VBE 2.x+ *only*

  } __attribute__((packed)) vbeInfoBlock;


  // (TODO: functions from Vbe.c)

  uint32 GetVbeInfoBlock(vbeInfoBlock* Buffer);


#endif
