# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# ...

.extern ErrorStubA
.extern ErrorStubB
.extern IrqStub

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

.globl IsrNoFault
.globl IsrDivideFault

.globl IsrDebug
.globl IsrOutOfBounds
.globl IsrInvalidOpcode

.globl IsrDeviceFault
.globl IsrDoubleFault

.globl IsrInvalidTss
.globl IsrSegmentFault

.globl IsrStackFault
.globl IsrGpFault
.globl IsrPageFault

.globl Isr87Fault

.globl IsrAlignCheck
.globl IsrMachineCheck

.globl IsrSimdFault
.globl IsrVirtFault

.globl IsrControlFault
.globl IsrHypervisorFault
.globl IsrVmmFault
.globl IsrSecurityFault

# ...

.globl Irq0
.globl Irq1
.globl Irq2
.globl Irq3
.globl Irq4
.globl Irq5
.globl Irq6
.globl Irq7
.globl Irq8
.globl Irq9
.globl Irq10
.globl Irq11
.globl Irq12
.globl Irq13
.globl Irq14
.globl Irq15


# ...
# Only faults related to interrupt numbers 10, 11, 12, 13, 14, and 17 use an error code.
# These are macros and they're the best thing known to man <3

# ...

.macro IsrFaultStub vNum

  pushal
  mov \vNum, %edx

  push %edx
  call ErrorStubA
  pop %edx

  popal
  iret

.endm

# ...

.macro IsrFaultStubWithError vNum

  pushal
  mov \vNum, %edx

  push %edx
  call ErrorStubA
  pop %edx

  popal
  pop %eax
  iret

.endm

# ...

.macro IsrAbortStub vNum

  pushal
  mov \vNum, %edx

  push %edx
  call ErrorStubB
  pop %edx

  popal
  iret

.endm

# ...

.macro IrqStubA vNum

  pushal
  mov \vNum, %edx

  push %edx
  call IrqStub
  pop %edx

  push %eax
  # inb $0x60, %al -> for keyboard
  mov $0x20, %al
  outb %al, $0x20
  pop %eax

  popal
  iret

.endm

# ...

.macro IrqStubB vNum

  pushal
  mov \vNum, %edx

  add $0x08, %edx
  push %edx
  call IrqStub
  pop %edx

  push %eax
  mov $0x20, %al
  outb %al, $0x20
  outb %al, $0xA0
  pop %eax

  popal
  iret

.endm


# ---

IsrNoFault:
  iret

# ---

IsrDivideFault:
  IsrAbortStub $0x00

IsrDebug:
  IsrFaultStub $0x01

IsrOutOfBounds:
  IsrFaultStub $0x05

IsrInvalidOpcode:
  IsrAbortStub $0x06

IsrDeviceFault:
  IsrFaultStub $0x07

# ---

IsrDoubleFault: # returns err (always 0)
  IsrAbortStub $0x08

IsrInvalidTss: # returns err
  IsrFaultStubWithError $0x0A

IsrSegmentFault: # returns err
  IsrFaultStubWithError $0x0B

IsrStackFault: # returns err
  IsrFaultStubWithError $0x0C

IsrGpFault: # returns err
  IsrFaultStubWithError $0x0D

IsrPageFault: # returns err
  IsrFaultStubWithError $0x0E

# ---

Isr87Fault:
  IsrFaultStub $0x10

# ---

IsrAlignCheck: # returns err
  IsrFaultStubWithError $0x11

IsrMachineCheck:
  IsrAbortStub $0x12

# ---

IsrSimdFault:
  IsrFaultStub $0x13

IsrVirtFault:
  IsrFaultStub $0x14

# ---

IsrControlFault: # returns err
  IsrFaultStubWithError $0x15

IsrHypervisorFault:
  IsrFaultStub $0x1C

IsrVmmFault: # returns err
  IsrFaultStubWithError $0x1D

IsrSecurityFault: # returns err
  IsrFaultStubWithError $0x1E

# ---

Irq0:
  IrqStubA $0x00

Irq1:
  IrqStubA $0x01

Irq2:
  IrqStubA $0x02

Irq3:
  IrqStubA $0x03

Irq4:
  IrqStubA $0x04

Irq5:
  IrqStubA $0x05

Irq6:
  IrqStubA $0x06

Irq7:
  IrqStubA $0x07

Irq8:
  IrqStubB $0x00

Irq9:
  IrqStubB $0x01

Irq10:
  IrqStubB $0x02

Irq11:
  IrqStubB $0x03

Irq12:
  IrqStubB $0x04

Irq13:
  IrqStubB $0x05

Irq14:
  IrqStubB $0x06

Irq15:
  IrqStubB $0x07
