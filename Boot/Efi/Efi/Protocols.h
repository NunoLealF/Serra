// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [Note] This header file is meant to provide a minimal implementation of UEFI
// for use within an x86-64 bootloader; as such, it doesn't implement anything
// beyond what would be needed for one, nor does it make an attempt to represent
// a standard or complete implementation of any UEFI specification.

#ifndef SERRA_EFI_PROTOCOLS_H
#define SERRA_EFI_PROTOCOLS_H

  // (Block IO Protocol-related definitions)

  typedef efiStatus (efiAbi *efiBlockRead) (efiProtocol This, uint32 MediaId, efiLba Lba, uint64 BufferSize, volatile void* Buffer);
  typedef efiStatus (efiAbi *efiBlockFlush) (efiProtocol This);
  constexpr efiUuid efiBlockIoProtocol_Uuid = {0x964E5B21, {0x6459, 0x11D2}, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

  typedef struct _efiBlockIoMedia {

    uint32 MediaId;

    bool RemovableMedia;
    bool MediaPresent;

    bool LogicalPartition;
    bool ReadOnly;
    bool WriteCaching;

    uint32 BlockSize;
    uint32 IoAlign;

    efiLba LastBlock;

  } efiBlockIoMedia;

  typedef struct _efiBlockIoProtocol {

    uint64 Revision;

    efiBlockIoMedia* Media;

    efiNotImplemented Reset;
    efiBlockRead ReadBlocks;
    efiNotImplemented WriteBlocks;
    efiBlockFlush FlushBlocks;

  } efiBlockIoProtocol;


  // (File Protocol-related definitions)

  typedef efiStatus (efiAbi *efiFileClose) (efiProtocol This);
  typedef efiStatus (efiAbi *efiFileGetInfo) (efiProtocol This, const efiUuid* InformationType, volatile uint64* BufferSize, volatile void* Buffer);
  typedef efiStatus (efiAbi *efiFileOpen) (efiProtocol This, volatile efiProtocol* NewHandle, char16* FileName, uint64 OpenMode, uint64 Attributes);
  typedef efiStatus (efiAbi *efiFileRead) (efiProtocol This, volatile uint64* BufferSize, volatile void* Buffer);

  typedef struct _efiFileProtocol {

    uint64 Revision;

    efiFileOpen Open;
    efiFileClose Close;

    efiNotImplemented Delete;
    efiFileRead Read;
    efiNotImplemented Write;

    efiNotImplemented GetPosition;
    efiNotImplemented SetPosition;

    efiFileGetInfo GetInfo;
    efiNotImplemented SetInfo;

    efiNotImplemented Flush;

  } efiFileProtocol;


  // (Graphics Output Protocol-related definitions)

  constexpr efiUuid efiEdidActiveProtocol_Uuid = {0xBD8C1056, {0x9F36, 0x44EC}, {0x92, 0xA8, 0xA6, 0x33, 0x7F, 0x81, 0x79, 0x86}};

  typedef struct _efiEdidProtocol {

    uint32 SizeOfEdid;
    uint8* Edid;

  } efiEdidProtocol;

  typedef struct _efiGraphicsOutputModeInformation {

    uint32 Version;

    uint32 HorizontalResolution;
    uint32 VerticalResolution;

    enum : uint32 {

      PixelRedGreenBlueReserved8BitPerColor,
      PixelBlueGreenRedReserved8BitPerColor,
      PixelBitMask,
      PixelBltOnly,
      PixelFormatMax

    } PixelFormat;

    struct {

      uint32 RedMask;
      uint32 GreenMask;
      uint32 BlueMask;

      uint32 ReservedMask;

    } PixelBitmask;

    uint32 PixelsPerScanline;

  } efiGraphicsOutputModeInformation;

  typedef struct _efiGraphicsOutputProtocolMode {

    uint32 MaxMode;
    uint32 Mode;

    efiGraphicsOutputModeInformation* Info;
    uint64 SizeOfInfo;

    efiPhysicalAddress FramebufferBase;
    uint64 FramebufferSize;

  } efiGraphicsOutputProtocolMode;

  typedef efiStatus (efiAbi *efiQueryMode) (efiProtocol This, uint32 ModeNumber, volatile uint64* SizeOfInfo, volatile efiGraphicsOutputModeInformation** EfiTable);
  typedef efiStatus (efiAbi *efiSetMode) (efiProtocol This, uint32 ModeNumber);
  constexpr efiUuid efiGraphicsOutputProtocol_Uuid = {0x9042A9DE, {0x23DC, 0x4A38}, {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}};

  typedef struct _efiGraphicsOutputProtocol {

    efiQueryMode QueryMode;
    efiSetMode SetMode;
    efiNotImplemented Blt;

    efiGraphicsOutputProtocolMode* Mode;

  } efiGraphicsOutputProtocol;


  // (Simple File System Protocol-related definitions)

  typedef efiStatus (efiAbi *efiOpenVolume) (efiProtocol This, volatile efiProtocol* Root);
  constexpr efiUuid efiSimpleFilesystemProtocol_Uuid = {0x0964E5B22, {0x6459, 0x11D2}, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

  typedef struct _efiSimpleFilesystemProtocol {

    uint64 Revision;
    efiOpenVolume OpenVolume;

  } efiSimpleFilesystemProtocol;


  // (Simple Text Output Protocol-related definitions)

  typedef efiStatus (efiAbi *efiReset) (efiProtocol This, bool ExtendedVerification);
  typedef efiStatus (efiAbi *efiClearScreen) (efiProtocol This);
  typedef efiStatus (efiAbi *efiSetAttribute) (efiProtocol This, uint64 Attribute);
  typedef efiStatus (efiAbi *efiOutputString) (efiProtocol This, char16* String);
  typedef efiStatus (efiAbi *efiTextQueryMode) (efiProtocol This, uint64 ModeNumber, uint64* Columns, uint64* Rows);
  typedef efiStatus (efiAbi *efiTextSetMode) (efiProtocol This, uint64 ModeNumber);

  typedef struct _efiSimpleTextOutputMode {

    int32 MaxMode;

    int32 Mode;
    int32 Attribute;

    int32 CursorColumn;
    int32 CursorRow;

    bool CursorVisible;

  } efiSimpleTextOutputMode;

  typedef struct _efiSimpleTextOutputProtocol {

    efiReset Reset;

    efiOutputString OutputString;
    efiNotImplemented TestString;

    efiTextQueryMode QueryMode;
    efiTextSetMode SetMode;
    efiSetAttribute SetAttribute;

    efiClearScreen ClearScreen;
    efiNotImplemented SetCursor;
    efiNotImplemented EnableCursor;

    efiSimpleTextOutputMode* Mode;

  } efiSimpleTextOutputProtocol;


  // (Simple Text Input Protocol-related definitions)

  typedef efiStatus (efiAbi *efiReadKeyStroke) (efiProtocol This, efiInputKey* Key);

  typedef struct _efiSimpleTextInputProtocol {

    efiReset Reset;
    efiReadKeyStroke ReadKeyStroke;

    efiEvent WaitForKey;

  } efiSimpleTextInputProtocol;

#endif
