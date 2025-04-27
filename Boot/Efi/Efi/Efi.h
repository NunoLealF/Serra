// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [Note] This header file is meant to provide a minimal implementation of UEFI
// for use within an x86-64 bootloader; as such, it doesn't implement anything
// beyond what would be needed for one, nor does it make an attempt to represent
// a standard or complete implementation of any UEFI specification.

#ifndef SERRA_EFI_H
#define SERRA_EFI_H

  // (Platform-specific definitions)

  // EFI uses the Microsoft ABI for C, *not* the System V ABI (which is used
  // by Linux, macOS, Unix, etc.), so we have to manually specify that.

  #define efiAbi __attribute__((ms_abi))


  // (Miscellaneous macros and definitions)

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
  typedef uint64 efiNotImplemented; // sizeof(uint64*) is still sizeof(void*), so this is okay

  typedef struct __efiUuid {

    uint32 Uuid_A;
    uint16 Uuid_B[2];
    uint8 Uuid_C[8];

  } efiUuid;


  // (Enums, and other types with a limited set of definitions)

  typedef enum __efiTpl : uint64 {

    TplApplication = 4,
    TplCallback = 8,
    TplNotify = 16,
    TplHighLevel = 31

  } efiTpl;

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


  // (Necessary table and protocol definitions)

  #include "Protocols.h"
  #include "Tables.h"

#endif
