// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_EFI_H
#define SERRA_EFI_H

  // (Miscellaneous definitions)

  #define efiAbi __attribute__((ms_abi)) // EFI uses the Microsoft/Windows ABI, not the System-V/Unix convention, so we have to specify that

  #define calculateRevision(Major, Minor) ((Major << 16) | Minor)
  #define errorBit (1ULL << 63)

  #define isSuccess(Status) (Status == 0)
  #define isError(Status) ((Status & errorBit) != 0)
  #define isWarning(Status) (!isSuccess(Status) && !isError(Status))

  typedef void* efiEvent;
  typedef void* efiHandle;
  typedef void* efiProtocol;

  typedef uint64 efiPhysicalAddress;
  typedef uint64 efiVirtualAddress;
  typedef uint64 efiNotImplemented; // sizeof(uint64*) is still sizeof(void*), so it's okay

  typedef struct __efiUuid {
    uint64 Data[2];
  } efiUuid;



  // (TPL (Task Priority Level) enum definition)

  typedef enum __efiTpl : uint64 {

    TplApplication = 4,
    TplCallback = 8,
    TplNotify = 16,
    TplHighLevel = 31

  } efiTpl;



  // (EFI Status enum definition)

  typedef enum __efiStatus : uint64 {

    // (Success status, all bits *clear*)

    EfiSuccess = 0,

    // (Warning statuses, highest bit *clear*)

    EfiWarnUnknownGlyph = 1,
    EfiWarnDeleteFailure = 2,
    EfiWarnWriteFailure = 3,
    EfiWarnBufferTooSmall = 4,
    EfiWarnStaleData = 5,
    EfiWarnFilesystem = 6,
    EfiWarnResetRequired = 7,

    // (Error statuses, highest bit *set*)

    EfiLoadError = (1 | errorBit),
    EfiInvalidParameter = (2 | errorBit),
    EfiUnsupported = (3 | errorBit),
    EfiBadBufferSize = (4 | errorBit),
    EfiBufferTooSmall = (5 | errorBit),
    EfiNotReady = (6 | errorBit),
    EfiDeviceError = (7 | errorBit),
    EfiWriteProtected = (8 | errorBit),
    EfiOutOfResources = (9 | errorBit),
    EfiVolumeCorrupted = (10 | errorBit),
    EfiVolumeFull = (11 | errorBit),
    EfiNoMedia = (12 | errorBit),
    EfiMediaChanged = (13 | errorBit),
    EfiNotFound = (14 | errorBit),
    EfiAccessDenied = (15 | errorBit),
    EfiNoResponse = (16 | errorBit),
    EfiNoMapping = (17 | errorBit),
    EfiTimeout = (18 | errorBit),
    EfiNotStarted = (19 | errorBit),
    EfiAlreadyStarted = (20 | errorBit),
    EfiAborted = (21 | errorBit),
    EfiIcmpError = (22 | errorBit),
    EfiTftpError = (23 | errorBit),
    EfiProtocolError = (24 | errorBit),
    EfiIncompatibleVersion = (25 | errorBit),
    EfiSecurityViolation = (26 | errorBit),
    EfiCrcError = (27 | errorBit),
    EfiEndOfMedia = (28 | errorBit),
    EfiEndOfFile = (31 | errorBit),
    EfiInvalidLanguage = (32 | errorBit),
    EfiCompromisedData = (33 | errorBit),
    EfiIpAddressConflict = (34 | errorBit),
    EfiHttpError = (35 | errorBit)

  } efiStatus;



  // (EFI Table Header)

  typedef struct __efiTableHeader {

    uint64 Signature; // Should be equal to the given signature.
    uint32 Revision;
    uint32 Size;

    uint32 Crc32;
    uint32 Reserved;

  } efiTableHeader;



  // (Simple Text Output Protocol)

  typedef efiStatus (efiAbi *efiClearScreen) (efiProtocol This);
  typedef efiStatus (efiAbi *efiSetAttribute) (efiProtocol This, uint64 Attribute);
  typedef efiStatus (efiAbi *efiOutputString) (efiProtocol This, char16* String);

  #define efiSimpleTextOutputProtocol_Uuid {0x11D269C7387477C2, 0x3B7269C9A000398E}

  typedef struct __efiSimpleTextOutputProtocol {

    efiNotImplemented Reset;

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



  // (Simple Text Input Protocol)

  #define efiSimpleTextInputProtocol_Uuid {0x11D269C7387477C1, 0x3B7269C9A000398E}

  typedef struct __efiSimpleTextInputProtocol {

    efiNotImplemented Reset;
    efiNotImplemented ReadKeyStroke;

    efiEvent WaitForKey;

  } efiSimpleTextInputProtocol;



  // (Boot Services table)

  typedef efiTpl (efiAbi *efiRaiseTpl) (efiTpl NewTpl);
  typedef void (efiAbi *efiRestoreTpl) (efiTpl OldTpl);
  typedef efiStatus (efiAbi *efiStall) (uint64 Microseconds);

  #define efiBootServicesSignature 0x56524553544F4F42

  typedef struct __efiBootServices {

    // Table header

    efiTableHeader Hdr;

    // TPL-related functions

    efiRaiseTpl RaiseTpl;
    efiRestoreTpl RestoreTpl;

    // Memory-related functions

    efiNotImplemented AllocatePages;
    efiNotImplemented FreePages;

    efiNotImplemented GetMemoryMap;

    efiNotImplemented AllocatePool;
    efiNotImplemented FreePool;

    // Timing-related functions

    efiNotImplemented CreateEvent;
    efiNotImplemented SetTimer;
    efiNotImplemented WaitForEvent;
    efiNotImplemented SignalEvent;
    efiNotImplemented CloseEvent;
    efiNotImplemented CheckEvent;

    // Protocol/handler-related functions

    efiNotImplemented InstallProtocolInterface;
    efiNotImplemented ReinstallProtocolInterface;
    efiNotImplemented UninstallProtocolInterface;
    efiNotImplemented HandleProtocol;
    void* Reserved2;

    efiNotImplemented RegisterProtocolNotify;
    efiNotImplemented LocateHandle;
    efiNotImplemented LocateDevicePath;
    efiNotImplemented InstallConfigurationTable;

    // Image-related functions

    efiNotImplemented LoadImage;
    efiNotImplemented StartImage;
    efiNotImplemented Exit;
    efiNotImplemented UnloadImage;

    efiNotImplemented ExitBootServices;

    // Miscellaneous functions

    efiNotImplemented GetNextMonotonicCount;
    efiStall Stall;
    efiNotImplemented SetWatchdogTimer;

    // Driver-related functions

    efiNotImplemented ConnectController;
    efiNotImplemented DisconnectController;

    // Protocol-related functions

    efiNotImplemented OpenProtocol;
    efiNotImplemented CloseProtocol;
    efiNotImplemented OpenProtocolInformation;

    efiNotImplemented ProtocolsPerHandle;
    efiNotImplemented LocateHandleBuffer;
    efiNotImplemented LocateProtocol;

    efiNotImplemented InstallMultipleProtocolInterfaces;
    efiNotImplemented UninstallMultipleProtocolInterfaces;

    // CRC-related functions

    efiNotImplemented CalculateCrc32;

    // More miscellaneous functions

    efiNotImplemented CopyMem;
    efiNotImplemented SetMem;
    efiNotImplemented CreateEventEx;

  } efiBootServices;



  // (System Table)

  #define efiSystemTableSignature 0x5453595320494249

  typedef struct __efiSystemTable {

    // Table header.

    efiTableHeader Hdr;

    // Everything else

    char16* FirmwareVendor;
    uint32 FirmwareRevision;

    efiHandle ConsoleInHandle;
    efiSimpleTextInputProtocol* ConIn;

    efiHandle ConsoleOutHandle;
    efiSimpleTextOutputProtocol* ConOut;

    efiHandle StandardErrorHandle;
    efiSimpleTextOutputProtocol* Stderr;

    efiNotImplemented* RuntimeServices; // efiRuntimeServices*
    efiBootServices* BootServices;

    uint64 NumberOfTableEntries;
    efiNotImplemented* ConfigurationTable; // efiConfigurationTable*

  } efiSystemTable;

#endif
