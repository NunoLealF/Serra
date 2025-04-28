// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [Note] This header file is meant to provide a minimal implementation of UEFI
// for use within an x86-64 bootloader; as such, it doesn't implement anything
// beyond what would be needed for one, nor does it make an attempt to represent
// a standard or complete implementation of any UEFI specification.

#ifndef SERRA_EFI_PROTOCOLS_H
#define SERRA_EFI_PROTOCOLS_H

  // (Graphics Output Protocol-related definitions)

  typedef struct __efiGraphicsOutputModeInformation {

    uint32 Version;

    uint32 HorizontalResolution;
    uint32 VerticalResolution;

    enum __efiGraphicsPixelFormat {

      PixelRedGreenBlueReserved8BitPerColor,
      PixelBlueGreenRedReserved8BitPerColor,
      PixelBitMask,
      PixelBltOnly,
      PixelFormatMax

    } PixelFormat;

    struct __efiPixelBitmask {

      uint32 RedMask;
      uint32 GreenMask;
      uint32 BlueMask;

      uint32 ReservedMask;

    } PixelBitmask;

    uint32 PixelsPerScanline;

  } efiGraphicsOutputModeInformation;

  typedef efiStatus (efiAbi *efiQueryMode) (efiProtocol This, uint32 ModeNumber, uint64* SizeOfInfo, void** EfiTable);
  typedef efiStatus (efiAbi *efiSetMode) (efiProtocol This, uint32 ModeNumber);

  #define efiGraphicsOutputProtocol_Uuid {0x9042A9DE, {0x23DC, 0x4A38}, {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}}

  typedef struct __efiGraphicsOutputProtocol {

    efiQueryMode QueryMode;
    efiSetMode SetMode;
    efiNotImplemented Blt;

    efiGraphicsOutputModeInformation* Mode;

  } efiGraphicsOutputProtocol;


  // (Simple Text Output Protocol-related definitions)

  typedef efiStatus (efiAbi *efiReset) (efiProtocol This, bool ExtendedVerification);
  typedef efiStatus (efiAbi *efiClearScreen) (efiProtocol This);
  typedef efiStatus (efiAbi *efiSetAttribute) (efiProtocol This, uint64 Attribute);
  typedef efiStatus (efiAbi *efiOutputString) (efiProtocol This, char16* String);

  typedef struct __efiSimpleTextOutputProtocol {

    efiReset Reset;

    efiOutputString OutputString;
    efiNotImplemented TestString;

    efiNotImplemented QueryMode;
    efiNotImplemented SetMode;
    efiSetAttribute SetAttribute;

    efiClearScreen ClearScreen;
    efiNotImplemented SetCursor;
    efiNotImplemented EnableCursor;

    efiNotImplemented* Mode;

  } efiSimpleTextOutputProtocol;


  // (Simple Text Input Protocol-related definitions)

  typedef efiStatus (efiAbi *efiReadKeyStroke) (efiProtocol This, efiInputKey* Key);

  typedef struct __efiSimpleTextInputProtocol {

    efiReset Reset;
    efiReadKeyStroke ReadKeyStroke;

    efiEvent WaitForKey;

  } efiSimpleTextInputProtocol;

#endif
