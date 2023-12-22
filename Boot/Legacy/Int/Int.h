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

  // ...

  typedef struct {

    uint16 OffsetLow;
    uint16 Selector;
    uint8 Reserved;
    uint8 Flags;
    uint16 OffsetHigh;

  } __attribute__((packed)) idtEntry;

  // ...

  extern void IsrNoFault(void);
  extern void IsrDivideFault(void);

  extern void IsrNmi(void);
  extern void IsrBreakpoint(void);
  extern void IsrOverflow(void);

  extern void IsrDebug(void);
  extern void IsrOutOfBounds(void);
  extern void IsrInvalidOpcode(void);

  extern void IsrCoprocessorOverrun(void);
  extern void IsrDeviceFault(void);
  extern void IsrDoubleFault(void);

  extern void IsrInvalidTss(void);
  extern void IsrSegmentFault(void);

  extern void IsrStackFault(void);
  extern void IsrGpFault(void);
  extern void IsrPageFault(void);

  extern void IsrReservedA(void);

  extern void Isr87Fault(void);
  extern void IsrAlignCheck(void);
  extern void IsrMachineCheck(void);

  extern void IsrSimdFault(void);
  extern void IsrVirtFault(void);

  extern void IsrReservedB(void);
  extern void IsrReservedC(void);
  extern void IsrReservedD(void);
  extern void IsrReservedE(void);
  extern void IsrReservedF(void);
  extern void IsrReservedG(void);

  extern void IsrControlFault(void);
  extern void IsrHypervisorFault(void);
  extern void IsrVmmFault(void);
  extern void IsrSecurityFault(void);

  extern void IsrReservedH(void);

  // god has abandoned us
  // IRQs 0-7 are on the master PIC (PIC A), while 8-15 are on the slave PIC (PIC B).

  extern void IrqTimer(void);
  extern void IrqKeyboard(void);
  extern void IrqCascade(void);
  extern void IrqCom2(void);
  extern void IrqCom1(void);
  extern void IrqLpt2(void);
  extern void IrqFloppy(void);
  extern void IrqLpt1(void);

  extern void IrqCmos(void);
  extern void IrqPeripheralA(void);
  extern void IrqPeripheralB(void);
  extern void IrqPeripheralC(void);
  extern void IrqMouse(void);
  extern void IrqFpu(void);
  extern void IrqHddA(void);
  extern void IrqHddB(void);

  // ...

  #define PicA_Data 0x20
  #define PicA_Command 0x21

  #define PicB_Data 0xA0
  #define PicB_Command 0xA1

  uint8 Inb(uint16 Port);
  void Outb(uint16 Port, uint8 Data);

  // ...

  void IrqHandler(uint8 Vector, uint8 Port);

  // ...

  void MaskPic(uint8 Mask);
  void InitPic(uint8 PicA_Offset, uint8 PicB_Offset);

  // ...

  void LoadIdt(descriptorTable* IdtDescriptor);
  void MakeIdtEntry(descriptorTable* IdtDescriptor, uint16 EntryNum, uint32 Offset, uint16 Selector, uint8 Gate, uint8 Dpl);

  void IsrFault(uint8 Vector, uint32 Eip);
  void IsrFaultWithError(uint8 Vector, uint32 Eip, uint32 Error);
  void IsrAbort(uint8 Vector, uint32 Eip);
  void IsrLog(uint8 Vector);

#endif
