// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_CPU_H
#define SERRA_CPU_H

  // (Structures from Cpu.c)

  typedef struct {

    uint32 Eax;
    uint32 Ebx;
    uint32 Ecx;
    uint32 Edx;

  } __attribute__ ((packed)) registerTable;

  typedef struct {

    // Available in all versions of ACPI (revision >= any)

    uint64 Signature;

    uint8 Checksum;
    uint8 Oem[6];
    uint8 Revision;

    uint32 Rsdt;

    // Available only in ACPI 2.x+ (revision >= 2)

    uint32 Length;
    uint64 Xsdt;

    uint8 ExtendedChecksum;
    uint8 Reserved[3];

  } __attribute__ ((packed)) acpiRsdpTable;


  // (Functions from Cpu.c)

  uint32 ReadEflags(void);
  void ChangeEflags(uint8 Bit, bool Set);

  bool SupportsCpuid(void);
  registerTable GetCpuid(uint32 Eax, uint32 Ecx);
  void GetVendorString(char* Buffer, registerTable Table);

  acpiRsdpTable* GetAcpiRsdpTable(void);

#endif
