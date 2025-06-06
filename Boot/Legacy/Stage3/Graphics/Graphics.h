// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_GRAPHICS_H
#define SERRA_GRAPHICS_H

  // Functions and data structures, from the (shared) Graphics folder

  #include "../../Shared/Graphics/Graphics.h"


  // VBE-related data structures, from Vbe.c

  typedef struct _vbeInfoBlock {

    // (VBE 1.0 and higher)

    uint32 Signature; // This should just be 'VESA', or 'VBE2'? I'm not sure
    uint16 Version; // Usually the high 8 bits are the overall version number, so, for example, VBE 1.1 is 0110h, and VBE 3.0 is 0300h

    farPtr OemStringPtr; // Far pointer to a null-terminated string
    uint32 Capabilities; // This is a bitfield (defined in the VESA spec)
    farPtr VideoModeListPtr; // Far pointer to a list of 16-byte mode numbers, where the last entry is FFFFh (to mark the end of the list)
    uint16 NumBlocks; // The number of memory blocks (64 KiB each)

    // (VBE 2.0 and higher)

    struct {

      uint16 VbeRevision;
      farPtr VendorNamePtr;
      farPtr ProductNamePtr;
      farPtr ProductRevPtr;

    } __attribute__((packed)) OemInfo;

    // (Reserved for future use)

    uint8 Reserved[222]; // Reserved for future VESA/VBE versions
    uint8 OemData[256]; // Reserved by the OEM; VBE 2.x+ *only*

  } __attribute__((packed)) vbeInfoBlock;

  typedef struct _vbeModeInfoBlock {

    // (VBE 1.0 and higher)

    uint16 ModeAttributes;

    struct {

      uint8 Attributes[2]; // First is window A, second is window B.
      uint16 Granularity;
      uint16 Size;
      uint16 Segments[2]; // First is window A, second is window B.

      uint32 FunctionPtr;

    } __attribute__((packed)) WindowInfo;

    // (VBE 1.2 and higher, except for ModeInfo.BytesPerScanLine)

    struct {

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

    struct {

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

    struct {

      uint32 Framebuffer; // Physical address of the framebuffer (if flat memory and not window)
      uint8 Reserved_Vbe2[6]; // The VBE 2 spec says this is used, while the VBE 3 spec says it isn't..?

    } __attribute__((packed)) Vbe2Info;

    // (VBE 3.0 and higher)

    struct {

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


  // EDID-related data structures, from Vbe.c

  typedef struct _edidDetailedTiming {

    struct {

      uint16 PixelClock; // The pixel clock, in 10 kHz steps, and with the upper/lower halves reversed..?

      uint16 HorizontalInfo_Low; // Upper half is resolution, lower half is blanking
      uint8 HorizontalInfo_High; // Upper half is resolution, lower half is blanking

      uint16 VerticalInfo_Low; // Upper half is resolution, lower half is blanking
      uint8 VerticalInfo_High; // Upper half is resolution, lower half is blanking

      uint16 HorizontalSyncData; // Upper half is offset/front porch, lower half is pulse width
      uint8 VerticalSyncData; // Upper half is offset/front porch, lower half is pulse width

      uint8 PixelSyncData; // Like the above, but smaller, and in pixels..?

    } __attribute__((packed)) Timings;

    struct {

      uint16 Size_Low; // Upper half is horizontal (low 8 bits), lower half is vertical (low 8 bits)
      uint8 Size_High; // Upper half is horizontal (high 4 bits), lower half is vertical (high 4 bits)

      uint8 HorizontalBorder; // Defined in the EDID standard
      uint8 VerticalBorder; // Defined in the EDID standard

      uint8 Flags; // Defined in the EDID standard

    } __attribute__((packed)) Display;

  } __attribute__((packed)) edidDetailedTiming;

  typedef struct _edidInfoBlock {

    uint64 Signature; // This should be 00FFFFFFFFFFFF00h

    uint8 VendorInfo[10]; // We don't need this for now, so we just keep it empty

    uint8 Version;
    uint8 Revision;

    uint8 DisplayInfo[5]; // Same as VendorInfo
    uint8 ColorInfo[10]; // Same as VendorInfo

    uint8 EstablishedTimings[3]; // Defined in the VESA specification
    uint16 StandardTimings[8]; // Defined in the VESA specification; different format though

    edidDetailedTiming DetailedTimings[4]; // Format defined in edidDetailedTiming (get your preferred resolution here)

    uint8 NumExtensionBlocks;
    uint8 Checksum; // (..check this?)

  } __attribute__((packed)) edidInfoBlock;


  // VBE- and EDID-related functions, from Vbe.c

  uint32 GetVbeInfoBlock(volatile vbeInfoBlock* Buffer);
  uint32 GetVbeModeInfo(volatile vbeModeInfoBlock* Buffer, uint16 ModeNumber);
  uint32 SetVbeMode(uint16 ModeNumber, bool UseCrtc, bool UseLinearModel, bool ClearDisplay, void* CrtcBuffer);

  uint32 GetEdidInfoBlock(volatile edidInfoBlock* Buffer, uint16 ControllerNum);

  uint16 FindBestVbeMode(uint16* VbeModeList, uint16 PreferredX_Resolution, uint16 PreferredY_Resolution);

#endif
