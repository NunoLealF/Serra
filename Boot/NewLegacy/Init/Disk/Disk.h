// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_DISK_H
#define SERRA_DISK_H

  // (int 13h, ah=48h)
  // https://www.ctyme.com/intr/rb-0715.htm#Table274

  // (We'll only actually work with / implement EDD 1.x, but just in case, we'll also reserve
  // space for EDD 2.x / EDD 3.x)

  typedef volatile struct {

    // [EDD 1.x, safe to use in any version]

    uint16 Size; // Call size of the buffer. (ALSO, SET THIS TO AT LEAST 42H FIRST!!)
    uint16 Flags; // Flags, see #0274

    uint32 NumPhysicalCylinders; // Number of physical cylinders
    uint32 NumPhysicalHeads; // Number of physical heads
    uint32 NumPhysicalSectors; // Number of physical sectors *per track*

    uint64 NumSectors; // Total number of sectors on the drive
    uint16 BytesPerSector; // Number of bytes per sector (can be 512, 2048, etc.!)

    // [EDD 2.x, not safe for versions below]

    uint8 Edd2_Data[4]; // Table #00278, or just FFFFFFFF if not available (?)

    // [EDD 3.x, not safe for versions below]

    uint8 Edd3_Data[36]; // ...

  } __attribute__((packed)) eddDriveParameters;

  /*

    Format of IBM/MS INT 13 Extensions drive parameters:

  Offset  Size    Description     (Table 00273)
  00h    WORD    (call) size of buffer
  (001Ah for v1.x, 001Eh for v2.x, 42h for v3.0)
  (ret) size of returned data
  02h    WORD    information flags (see #00274)
  04h    DWORD   number of physical cylinders on drive
  08h    DWORD   number of physical heads on drive
  0Ch    DWORD   number of physical sectors per track
  10h    QWORD   total number of sectors on drive
  18h    WORD    bytes per sector
  ---v2.0+ ---
  1Ah    DWORD   -> EDD configuration parameters (see #00278)
  FFFFh:FFFFh if not available
  ---v3.0 ---
  1Eh    WORD    signature BEDDh to indicate presence of Device Path info
  20h    BYTE    length of Device Path information, including signature and this
  byte (24h for v3.0)
  21h  3 BYTEs   reserved (0)
  24h  4 BYTEs   ASCIZ name of host bus ("ISA" or "PCI")
  28h  8 BYTEs   ASCIZ name of interface type
  "ATA"
  "ATAPI"
  "SCSI"
  "USB"
  "1394" IEEE 1394 (FireWire)
  "FIBRE" Fibre Channel
  30h  8 BYTEs   Interface Path (see #00275)
  38h  8 BYTEs   Device Path (see #00276)
  40h    BYTE    reserved (0)
  41h    BYTE    checksum of bytes 1Eh-40h (two's complement of sum, which makes
  the 8-bit sum of bytes 1Eh-41h equal 00h)

  */

  // (Todo: other functions)

#endif
