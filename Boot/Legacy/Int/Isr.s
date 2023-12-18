# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# ...

.extern ErrorStubA
.extern ErrorStubB

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

.globl IsrNoErrorStub
.globl IsrFaultStub
.globl IsrAbortStub

# don't forget to add the rest

.globl IsrDoubleFault
.globl IsrGpFault
.globl IsrDivideFault

# Error stubs.

IsrNoErrorStub:
  iret

IsrFaultStub:
  call ErrorStubA
  iret

IsrAbortStub:
  call ErrorStubB
  iret

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
  IsrFaultStub $0x06

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
