// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_CPU_H
#define SERRA_CPU_H

  // (Structures from Cpu.c)

  typedef struct __registerTable {

    uint32 Eax;
    uint32 Ebx;
    uint32 Ecx;
    uint32 Edx;

  } __attribute__ ((packed)) registerTable;

  typedef struct __acpiRsdpTable {

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

  typedef struct __pciBiosInfoTable {

    uint32 Signature; // This should be set to 'PCI', or 20494350h

    uint8 Characteristics; // Bitfield
    uint8 InterfaceLevel[2]; // First is major version, second is minor version

    uint8 LastPciBus; // Last PCI bus in system (it's normal for this to return 00h!)

  } __attribute__((packed)) pciBiosInfoTable;


  // (Functions and definitions from Cpu.c)

  uint32 ReadEflags(void);
  void ChangeEflags(uint8 Bit, bool Set);

  bool SupportsCpuid(void);
  registerTable GetCpuid(uint32 Eax, uint32 Ecx);
  void GetVendorString(char* Buffer, registerTable Table);

  acpiRsdpTable* GetAcpiRsdpTable(void);
  void* GetSmbiosEntryPointTable(void);
  uint32 GetPciBiosInfoTable(pciBiosInfoTable* PciBiosTable);

  #define patMsr 0x277
  #define longModeMsr 0xC0000080

  void WriteToMsr(uint32 Msr, uint64 Value);
  uint64 ReadFromMsr(uint32 Msr);

  void WriteToControlRegister(uint8 Register, uint32 Value);
  uint32 ReadFromControlRegister(uint8 Register);

  // Macros, for use with bitwise operations.

  #define clearBit(Value, Bit) (Value & (typeof(Value))(~(1ULL << Bit)))
  #define setBit(Value, Bit) (Value | (typeof(Value))((1ULL << Bit)))

#endif
