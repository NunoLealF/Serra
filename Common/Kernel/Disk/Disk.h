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

      uint8 DriveNumber; // (The drive number we need to pass on, in DL)
      bool EddSupported; // (Are EDD extensions to int 13h supported?)

      uint16 BytesPerSector; // (The number of bytes per sector - *EDD only, or 512*)
      uint64 NumSectors; // (The total number of sectors on the drive - *EDD only*)

    } __attribute__((packed)) Int13;

  } diskInfo;

  // Include functions and global variables from Disk.c

  extern diskInfo DiskInfo;
  bool InitializeDiskSubsystem(void* InfoTable);

  // Include functions and global variables from Fs.c (TODO)

#endif
