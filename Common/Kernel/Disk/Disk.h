// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_DISK_H
#define SERRA_KERNEL_DISK_H

  // Import standard and/or necessary headers.

  #include "../Libraries/Stdint.h"

  #include "Bios/Bios.h"
  #include "Efi/Efi.h"

  // Include data structures from Disk.c (TODO)

  typedef struct _diskInfo {

    // (Has this subsystem been initialized yet?)

    bool IsSupported;

    // (Which method did we boot with - and can be guaranteed to work?)

    enum {

      BootMethod_Unknown = 0,
      BootMethod_Efi = 1,
      BootMethod_Int13 = 2

    } BootMethod;

    // (EFI Boot Services specific information)

    struct {

      const void* Handle; // (The efiSimpleFilesystemProtocol handle we used)
      const void* Protocol; // (A pointer to the efiFileProtocol we used)

      const void* FileInfo; // (A pointer to our kernel file's efiFileInfo table)

    } __attribute__((packed)) Efi;

    // (BIOS / Int 13h specific information)

    struct {

      uint8 DriveNumber; // (The drive number we need to pass on, in `dl`)

      uint16 BytesPerSector; // (The number of bytes per sector)
      uint64 NumSectors; // (The number of sectors)

    } __attribute__((packed)) Int13;

  } diskInfo;

  // Include functions and global variables from Disk.c

  bool InitializeDiskSubsystem(void* InfoTable);

  // Include functions and global variables from Fs.c (TODO)

#endif
