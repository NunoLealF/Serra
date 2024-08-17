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


  // VBE-related data structures. (Vbe.c)

  typedef struct {

    // (VBE 1.0 and higher)

    uint32 Signature; // This should just be 'VESA', or 'VBE2'? I'm not sure
    uint16 Version; // Usually the high 8 bits are the overall version number, so, for example, VBE 1.1 is 1010h, and VBE 3.0 is 3000h

    farPtr OemStringPtr; // Far pointer to a null-terminated string
    uint32 Capabilities; // This is a bitfield (defined in the VESA spec)
    farPtr VideoModeListPtr; // Far pointer to a list of 16-byte mode numbers, where the last entry is FFFFh (to mark the end of the list)
    uint16 NumBlocks; // The number of memory blocks (64 KiB each)

    // (VBE 2.0 and higher)

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


  typedef struct {

    // (VBE 1.0 and higher)

    uint16 ModeAttributes;

    struct __WindowInfo {

      uint8 Attributes[2]; // First is window A, second is window B.
      uint16 Granularity;
      uint16 Size;
      uint16 Segments[2]; // First is window A, second is window B.

      uint32 FunctionPtr;

    } __attribute__((packed)) WindowInfo;

    // (VBE 1.2 and higher, except for ModeInfo.BytesPerScanLine)

    struct __ModeInfo {

      uint16 BytesPerScanLine; // (supported by VBE 1.0!)

      uint16 X_Resolution;
      uint16 Y_Resolution;

      uint8 X_CharSize;
      uint8 Y_CharSize;

      uint8 NumPlanes;
      uint8 BitsPerPixel;
      uint8 NumBanks;

      uint8 MemoryModel;
      uint8 BankSize; // (in KiB)
      uint8 NumImagePages;
      uint8 Reserved_Vbe1; // set to 1..?

    } __attribute__((packed)) ModeInfo;

    struct __ColorInfo {

      uint8 RedMaskSize;
      uint8 RedBit;

      uint8 GreenMaskSize;
      uint8 GreenBit;

      uint8 BlueMaskSize;
      uint8 BlueBit;

      uint8 ReservedMaskSize;
      uint8 ReservedBit;

      uint8 ColorModeInfo;

    } __attribute__((packed)) ColorInfo;

    // (VBE 2.0 and higher)

    struct __Vbe2Info {

      uint32 Framebuffer; // Physical address of the framebuffer (if flat memory and not window)
      uint8 Reserved_Vbe2[6]; // Always set to 0

    } __attribute__((packed)) Vbe2Info;

    // (VBE 3.0 and higher)

    struct __Vbe3Info {

      uint16 LinearBytesPerScanLine;

      uint8 BankedNumImagePages;
      uint8 LinearNumImagePages;

      uint8 LinearRedMaskSize;
      uint8 LinearRedBit;

      uint8 LinearGreenMaskSize;
      uint8 LinearGreenBit;

      uint8 LinearBlueMaskSize;
      uint8 LinearBlueBit;

      uint8 LinearReservedMaskSize;
      uint8 LinearReservedBit;

      uint32 MaxPixelClock;

    } __attribute__((packed)) Vbe3Info;

    // (Reserved)

    uint8 Reserved[190]; // The VBE spec says 189 bytes, but I'm pretty sure it's 190..?

  } __attribute__((packed)) vbeModeInfoBlock;


  // (TODO: functions from Vbe.c)

  uint32 GetVbeInfoBlock(vbeInfoBlock* Buffer);
  uint32 GetVbeModeInfo(vbeModeInfoBlock* Buffer, uint16 ModeNumber);
  uint32 SetVbeMode(uint16 ModeNumber, bool UseCrtc, bool UseLinearModel, bool ClearDisplay, void* CrtcBuffer);


#endif
