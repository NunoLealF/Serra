// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [Note] This header file is meant to provide a minimal implementation of UEFI
// for use within an x86-64 bootloader; as such, it doesn't implement anything
// beyond what would be needed for one, nor does it make an attempt to represent
// a standard or complete implementation of any UEFI specification.

#ifndef SERRA_EFI_TABLES_H
#define SERRA_EFI_TABLES_H

  // (General table-related definitions)

  typedef struct _efiTableHeader {

    uint64 Signature;
    uint32 Revision;
    uint32 Size;

    uint32 Crc32;
    uint32 Reserved;

  } efiTableHeader;


  // (Boot Services-related definitions)

  #define GetMmapEntry(Mmap, DescriptorSize, Index) (efiMemoryDescriptor*)((uint64)Mmap + (DescriptorSize * Index))

  typedef efiStatus (efiAbi *efiAllocatePool) (efiMemoryType PoolType, uint64 Size, volatile void** Buffer);
  typedef efiStatus (efiAbi *efiAllocatePages) (efiAllocateType Type, efiMemoryType MemoryType, uint64 Pages, volatile efiPhysicalAddress* Memory);
  typedef efiStatus (efiAbi *efiCloseProtocol) (efiHandle Handle, const efiUuid* Protocol, efiHandle AgentHandle, efiHandle ControllerHandle);
  typedef efiStatus (efiAbi *efiExit) (efiHandle ImageHandle, efiStatus ExitStatus, uint64 ExitDataSize, char16* ExitData);
  typedef efiStatus (efiAbi *efiFreePool) (void* Buffer);
  typedef efiStatus (efiAbi *efiFreePages) (efiPhysicalAddress Memory, uint64 Pages);
  typedef efiStatus (efiAbi *efiGetMemoryMap) (volatile uint64* MemoryMapSize, volatile efiMemoryDescriptor* MemoryMap, volatile uint64* MapKey, volatile uint64* DescriptorSize, volatile uint32* DescriptorVersion);
  typedef efiStatus (efiAbi *efiLocateHandle) (efiLocateSearchType SearchType, const efiUuid* Protocol, void* SearchKey, uintptr* BufferSize, efiHandle* Buffer);
  typedef efiStatus (efiAbi *efiLocateHandleBuffer) (efiLocateSearchType SearchType, const efiUuid* Protocol, void* SearchKey, uintptr* NoHandles, efiHandle** Buffer);
  typedef efiStatus (efiAbi *efiLocateProtocol) (const efiUuid* Protocol, void* Registration, efiProtocol* Interface);
  typedef efiStatus (efiAbi *efiOpenProtocol) (efiHandle Handle, const efiUuid* Protocol, efiProtocol* Interface, efiHandle AgentHandle, efiHandle ControllerHandle, uint32 Attributes);
  typedef efiTpl (efiAbi *efiRaiseTpl) (efiTpl NewTpl);
  typedef void (efiAbi *efiRestoreTpl) (efiTpl OldTpl);

  #define efiBootServicesSignature 0x56524553544F4F42

  typedef struct _efiBootServices {

    // (Table header; always check this first.)

    efiTableHeader Hdr;

    // (TPL-related functions)

    efiRaiseTpl RaiseTpl;
    efiRestoreTpl RestoreTpl;

    // (Memory-related functions)

    efiAllocatePages AllocatePages;
    efiFreePages FreePages;

    efiGetMemoryMap GetMemoryMap;

    efiAllocatePool AllocatePool;
    efiFreePool FreePool;

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
    efiLocateHandle LocateHandle;
    efiNotImplemented LocateDevicePath;
    efiNotImplemented InstallConfigurationTable;

    // (Image-related functions)

    efiNotImplemented LoadImage;
    efiNotImplemented StartImage;
    efiExit Exit;
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

    efiOpenProtocol OpenProtocol;
    efiCloseProtocol CloseProtocol;
    efiNotImplemented OpenProtocolInformation;

    efiNotImplemented ProtocolsPerHandle;
    efiLocateHandleBuffer LocateHandleBuffer;
    efiLocateProtocol LocateProtocol;

    efiNotImplemented InstallMultipleProtocolInterfaces;
    efiNotImplemented UninstallMultipleProtocolInterfaces;

    // (CRC-related functions)

    efiNotImplemented CalculateCrc32;

    // (More miscellaneous functions)

    efiNotImplemented CopyMem;
    efiNotImplemented SetMem;

  } efiBootServices;


  // (File Info-related definitions)

  constexpr efiUuid efiFileInfo_Uuid = {0x09576E92, {0x6D3F, 0x11D2}, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

  typedef struct _efiFileInfo {

    uint64 Size;
    uint64 FileSize;
    uint64 PhysicalSize;

    // Other entries are ignored

  } efiFileInfo;


  // (Runtime Services-related definitions)

  #define efiRuntimeServicesSignature 0x56524553544E5552

  typedef struct _efiRuntimeServices {

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


  // (Configuration Table-related definitions)

  constexpr efiUuid efiAcpiTable_Uuid = {0xEB9D2D30, {0x2D88, 0x11D3}, {0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
  constexpr efiUuid efiAcpi2Table_Uuid = {0x8868E871, {0xE4F1, 0x11D3}, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};

  constexpr efiUuid efiSmbiosTable_Uuid = {0xEB9D2D31, {0x2D88, 0x11D3}, {0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
  constexpr efiUuid efiSmbios3Table_Uuid = {0xF2FD1544, {0x9794, 0x4A2C}, {0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94}};

  typedef struct _efiConfigurationTable {

    efiUuid VendorGuid;
    void* VendorTable;

  } efiConfigurationTable;


  // (System Table-related definitions)

  #define efiSystemTableSignature 0x5453595320494249

  typedef struct _efiSystemTable {

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

    efiRuntimeServices* RuntimeServices;
    efiBootServices* BootServices;

    // (Configuration table)

    uint64 NumberOfTableEntries;
    efiConfigurationTable* ConfigurationTable;

  } efiSystemTable;

#endif
