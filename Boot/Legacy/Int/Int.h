// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_INT_H
#define SERRA_INT_H

  // Todo - everything

  // ...

  typedef struct {

    uint16 Size;
    uint32 Offset;

  } __attribute__((packed)) descriptorTable;

  /// ...

  typedef struct {

    uint16 OffsetLow;
    uint16 Selector;
    uint8 Reserved;
    uint8 Flags;
    uint16 OffsetHigh;

  } __attribute__((packed)) idtEntry;

  // ...

  extern void IsrNoErrorStub(void);
  extern void IsrFaultStub(void);
  extern void IsrAbortStub(void);

  // ...

  #define PicA_Data 0x20
  #define PicA_Command 0x21

  #define PicB_Data 0xA0
  #define PicB_Command 0xA1

  uint8 Inb(uint16 Port);
  void Outb(uint16 Port, uint8 Data);

  void MaskPic(uint8 Mask);
  void InitPic(uint8 PicA_Offset, uint8 PicB_Offset);

  // ...

  void LoadIdt(descriptorTable* IdtDescriptor);
  void MakeIdtEntry(descriptorTable* IdtDescriptor, uint16 EntryNum, uint32 Offset, uint16 Selector, uint8 Gate, uint8 Dpl);

#endif
