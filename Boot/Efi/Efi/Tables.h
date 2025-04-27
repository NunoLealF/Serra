// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [Note] This header file is meant to provide a minimal implementation of UEFI
// for use within an x86-64 bootloader; as such, it doesn't implement anything
// beyond what would be needed for one, nor does it make an attempt to represent
// a standard or complete implementation of any UEFI specification.

#ifndef SERRA_EFI_TABLES_H
#define SERRA_EFI_TABLES_H

  // (Table-related definitions)

  typedef struct __efiTableHeader {

    uint64 Signature;
    uint32 Revision;
    uint32 Size;

    uint32 Crc32;
    uint32 Reserved;

  } efiTableHeader;


  // (Boot Services-related definitions)

  typedef efiTpl (efiAbi *efiRaiseTpl) (efiTpl NewTpl);
  typedef void (efiAbi *efiRestoreTpl) (efiTpl OldTpl);
  typedef efiStatus (efiAbi *efiLocateProtocol) (efiUuid* Protocol, void* Registration, void** Interface);

  #define efiBootServicesSignature 0x56524553544F4F42

  typedef struct __efiBootServices {

    // (Table header; always check this first.)

    efiTableHeader Hdr;

    // (TPL-related functions)

    efiRaiseTpl RaiseTpl;
    efiRestoreTpl RestoreTpl;

    // (Memory-related functions)

    efiNotImplemented AllocatePages;
    efiNotImplemented FreePages;

    efiNotImplemented GetMemoryMap;

    efiNotImplemented AllocatePool;
    efiNotImplemented FreePool;

    // (Timing-related functions)

    efiNotImplemented CreateEvent;
    efiNotImplemented SetTimer;
    efiNotImplemented WaitForEvent;
    efiNotImplemented SignalEvent;
    efiNotImplemented CloseEvent;
    efiNotImplemented CheckEvent;

    // (Protocol/handler-related functions)

    efiNotImplemented InstallProtocolInterface;
    efiNotImplemented ReinstallProtocolInterface;
    efiNotImplemented UninstallProtocolInterface;
    efiNotImplemented HandleProtocol;
    void* AlsoReserved;

    efiNotImplemented RegisterProtocolNotify;
    efiNotImplemented LocateHandle;
    efiNotImplemented LocateDevicePath;
    efiNotImplemented InstallConfigurationTable;

    // (Image-related functions)

    efiNotImplemented LoadImage;
    efiNotImplemented StartImage;
    efiNotImplemented Exit;
    efiNotImplemented UnloadImage;

    efiNotImplemented ExitBootServices;

    // (Miscellaneous functions)

    efiNotImplemented GetNextMonotonicCount;
    efiNotImplemented Stall;
    efiNotImplemented SetWatchdogTimer;

    // (Driver-related functions)

    efiNotImplemented ConnectController;
    efiNotImplemented DisconnectController;

    // (Protocol-related functions)

    efiNotImplemented OpenProtocol;
    efiNotImplemented CloseProtocol;
    efiNotImplemented OpenProtocolInformation;

    efiNotImplemented ProtocolsPerHandle;
    efiNotImplemented LocateHandleBuffer;
    efiLocateProtocol LocateProtocol;

    efiNotImplemented InstallMultipleProtocolInterfaces;
    efiNotImplemented UninstallMultipleProtocolInterfaces;

    // (CRC-related functions)

    efiNotImplemented CalculateCrc32;

    // (More miscellaneous functions)

    efiNotImplemented CopyMem;
    efiNotImplemented SetMem;

  } efiBootServices;


  // (Runtime Services-related definitions)

  #define efiRuntimeServicesSignature 0x56524553544E5552

  typedef struct __efiRuntimeServices {

    // (Table header; always check this first.)

    efiTableHeader Hdr;

    // (Time-related functions)

    efiNotImplemented GetTime;
    efiNotImplemented SetTime;

    efiNotImplemented GetWakeupTime;
    efiNotImplemented SetWakeupTime;

    // (Virtual memory-related functions)

    efiNotImplemented SetVirtualAddressMap;
    efiNotImplemented ConvertPointer;

    // (Variable-related functions)

    efiNotImplemented GetVariable;
    efiNotImplemented GetNextVariableName;
    efiNotImplemented SetVariable;

    // (Miscellaneous functions)

    efiNotImplemented GetNextHighMonotonicCount;
    efiNotImplemented ResetSystem;

  } efiRuntimeServices;


  // (System Table-related definitions)

  #define efiSystemTableSignature 0x5453595320494249

  typedef struct __efiSystemTable {

    // (Table header; always check this first.)

    efiTableHeader Hdr;

    // (Firmware information)

    char16* FirmwareVendor;
    uint32 FirmwareRevision;

    // (Console functions and handles)

    efiHandle ConsoleInHandle;
    efiSimpleTextInputProtocol* ConIn;

    efiHandle ConsoleOutHandle;
    efiSimpleTextOutputProtocol* ConOut;

    efiHandle StandardErrorHandle;
    efiSimpleTextOutputProtocol* Stderr;

    // (Boot and runtime services)

    efiNotImplemented* RuntimeServices; // efiRuntimeServices*
    efiBootServices* BootServices;

    // (Configuration table)

    uint64 NumberOfTableEntries;
    efiNotImplemented* ConfigurationTable; // efiConfigurationTable*

  } efiSystemTable;

#endif
