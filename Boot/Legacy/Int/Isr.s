# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# ...

.extern IsrFault
.extern IsrFaultWithError
.extern IsrAbort
.extern IsrLog

.extern IrqHandler

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

.globl IsrNoFault
.globl IsrDivideFault

.globl IsrNmi
.globl IsrBreakpoint
.globl IsrOverflow
.globl IsrDebug

.globl IsrOutOfBounds
.globl IsrInvalidOpcode

.globl IsrDeviceFault
.globl IsrDoubleFault
.globl IsrCoprocessorOverrun

.globl IsrInvalidTss
.globl IsrSegmentFault

.globl IsrStackFault
.globl IsrGpFault
.globl IsrPageFault

.globl IsrReservedA
.globl Isr87Fault

.globl IsrAlignCheck
.globl IsrMachineCheck

.globl IsrSimdFault
.globl IsrVirtFault

.globl IsrReservedB
.globl IsrReservedC
.globl IsrReservedD
.globl IsrReservedE
.globl IsrReservedF
.globl IsrReservedG

.globl IsrControlFault
.globl IsrHypervisorFault
.globl IsrVmmFault
.globl IsrSecurityFault

.globl IsrReservedH

# ...

.globl IrqTimer
.globl IrqKeyboard
.globl IrqCascade
.globl IrqCom2
.globl IrqCom1
.globl IrqLpt2
.globl IrqFloppy
.globl IrqLpt1

.globl IrqCmos
.globl IrqPeripheralA
.globl IrqPeripheralB
.globl IrqPeripheralC
.globl IrqMouse
.globl IrqFpu
.globl IrqHddA
.globl IrqHddB


# ...
# Only faults related to interrupt numbers 10, 11, 12, 13, 14, and 17 use an error code.
# These are macros and they're the best thing known to man <3

# ...

.macro IsrFaultStub vNum

  # get eip

  push %ecx
  mov 4(%esp), %ecx

  # ...

  pushal
  mov \vNum, %edx

  # EIP is already pushed to the stack

  push %ecx
  push %edx

  call IsrFault

  pop %edx
  pop %ecx

  popal
  pop %ecx
  iret

.endm


# ...

.macro IsrFaultStubWithError vNum

  # get eip/error

  push %ebx
  push %ecx

  mov 12(%esp), %ebx
  mov 8(%esp), %ecx

  # ...

  pushal
  mov \vNum, %edx

  # EIP is already pushed to the stack

  push %ebx
  push %ecx
  push %edx

  call IsrFaultWithError

  pop %edx
  pop %ecx
  pop %ebx

  # ...

  popal
  pop %ebx
  pop %ecx
  iret

.endm


# ...

.macro IsrAbortStub vNum

  # get eip

  push %ecx
  mov 4(%esp), %ecx

  # ...

  pushal
  mov \vNum, %edx

  # EIP is already pushed to the stack

  push %ecx
  push %edx

  call IsrAbort

  pop %edx
  pop %ecx

  # ...

  popal
  pop %ecx
  iret

.endm

# ...

.macro IsrLogStub vNum

  pushal
  mov \vNum, %edx

  # ...

  push %edx
  call IsrLog
  pop %edx

  # ...

  popal
  iret

.endm

# ...

.macro IrqStub vNum pNum

  pushal

  mov \vNum, %edx
  mov \pNum, %ecx

  push %ecx
  push %edx

  call IrqHandler

  pop %edx
  pop %ecx

  # ...

  mov $0x20, %al
  outb %al, $0x20
  outb %al, $0xA0

  popal
  iret

.endm


# ---

IsrNoFault:
  iret

# ---

IsrDivideFault:
  IsrAbortStub $0

IsrDebug:
  IsrFaultStub $1

IsrNmi:
  IsrLogStub $2

IsrBreakpoint:
  IsrLogStub $3

IsrOverflow:
  IsrFaultStub $4

IsrOutOfBounds:
  IsrFaultStub $5

IsrInvalidOpcode:
  IsrAbortStub $6

IsrDeviceFault:
  IsrFaultStub $7

# ---

IsrDoubleFault: # returns err (always 0)
  IsrAbortStub $8

IsrCoprocessorOverrun:
  IsrFaultStub $9

IsrInvalidTss: # returns err
  IsrFaultStubWithError $10

IsrSegmentFault: # returns err
  IsrFaultStubWithError $11

IsrStackFault: # returns err
  IsrFaultStubWithError $12

IsrGpFault: # returns err
  IsrFaultStubWithError $13

IsrPageFault: # returns err
  IsrFaultStubWithError $14

# ---

IsrReservedA:
  IsrLogStub $15

# ---

Isr87Fault:
  IsrFaultStub $16

# ---

IsrAlignCheck: # returns err
  IsrFaultStubWithError $17

IsrMachineCheck:
  IsrAbortStub $18

# ---

IsrSimdFault:
  IsrFaultStub $19

IsrVirtFault:
  IsrFaultStub $20

IsrControlFault: # returns err
  IsrFaultStubWithError $21

# ---

IsrReservedB:
  IsrLogStub $22

IsrReservedC:
  IsrLogStub $23

IsrReservedD:
  IsrLogStub $24

IsrReservedE:
  IsrLogStub $25

IsrReservedF:
  IsrLogStub $26

IsrReservedG:
  IsrLogStub $27

# ---

IsrHypervisorFault:
  IsrFaultStub $28

IsrVmmFault: # returns err
  IsrFaultStubWithError $29

IsrSecurityFault: # returns err
  IsrFaultStubWithError $30

# ---

IsrReservedH:
  IsrLogStub $31

# --- (IRQ-A, master PIC, 0x20->0x27 (0 to 7))

IrqTimer:
  IrqStub $0 $0x00

IrqKeyboard:
  IrqStub $1 $0x60

IrqCascade:
  IrqStub $2 $0x00

IrqCom2:
  IrqStub $3 $0x00

IrqCom1:
  IrqStub $4 $0x00

IrqLpt2:
  IrqStub $5 $0x00

IrqFloppy:
  IrqStub $6 $0x00

IrqLpt1:
  IrqStub $7 $0x00

# --- (IRQ-B, slave PIC, 0x28->0x2F (8 to 15))

IrqCmos:
  IrqStub $8 $0x00

IrqPeripheralA:
  IrqStub $9 $0x00

IrqPeripheralB:
  IrqStub $10 $0x00

IrqPeripheralC:
  IrqStub $11 $0x00

IrqMouse:
  IrqStub $12 $0x00

IrqFpu:
  IrqStub $13 $0x00

IrqHddA:
  IrqStub $14 $0x00

IrqHddB:
  IrqStub $15 $0x00
