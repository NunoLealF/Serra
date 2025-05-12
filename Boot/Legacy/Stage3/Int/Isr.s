# Copyright (C) 2025 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# There are functions (defined elsewhere in our bootloader) we want to have access to, so we
# use the .extern keyword.

.extern IsrFault
.extern IsrFaultWithError
.extern IsrAbort
.extern IsrLog

.extern IrqHandler


# We want to be able to reference the functions in here from the rest of our bootloader, so we
# use .globl to turn them into global functions that can be called from anywhere else.

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


# (macro) IsrFaultStub()
#
# Inputs: \vNum/%edx - The specific vector number of our interrupt.
#         (%eip/ecx - The address of the instruction that caused the exception.)
#
# Outputs: (None)
#
# This macro essentially serves as a template ISR (interrupt service handler) / 'handler' for
# exceptions that are problematic, but that aren't unrecoverable, and that *don't* provide an
# error code (if they do, use IsrFaultStubWithError).
#
# It takes one explicit argument ('vNum', the vector/interrupt number), and one implicit
# argument (%eip/ecx, the instruction pointer at the time of the exception, which has already
# been pushed onto the stack by our firmware).
#
# After that, it calls IsrFault(), which is defined in Int.c.

.macro IsrFaultStub vNum

  # Push %ecx to the stack (to save the original value of it), and move the instruction
  # pointer of the instruction that caused the exception to %ecx.

  push %ecx
  mov 4(%esp), %ecx

  # Push everything to the stack (including %ecx), and change the value of %edx to the given
  # vector number (vNum).

  pushal
  mov \vNum, %edx

  # Call the IsrFault() function with two arguments - %ecx (%eip), and %edx (vNum).

  push %ecx
  push %edx

  call IsrFault

  pop %edx
  pop %ecx

  # Pop everything from the stack (and then pop %ecx), and finally, after everything, use
  # the 'iret' instruction to return from our ISR.

  popal
  pop %ecx
  iret

.endm


# (macro) IsrFaultStubWithError()
#
# Inputs: \vNum/%edx - The specific vector number of our interrupt.
#         (%eip/ebx - The address of the instruction that caused the exception.)
#         (%ecx - The error code pushed to the stack by the exception.)
#
# Outputs: (None)
#
# This macro essentially serves as a template ISR (interrupt service handler) / 'handler' for
# exceptions that are problematic, but that aren't unrecoverable, and that *do* provide an
# error code (if they don't, use IsrFaultStub).
#
# It takes one explicit argument ('vNum', the vector/interrupt number), and two implicit
# arguments (%eip/ebx, the instruction pointer at the time of the exception, and %ecx, the
# error code pushed to the stack by the exception).
#
# After that, it calls IsrFaultWithError(), which is defined in Int.c.

.macro IsrFaultStubWithError vNum

  # Push %ebx and %ecx to the stack (to save the original value of it), and move the error code
  # and the instruction pointer of the instruction that caused the exception to %ecx and %ebx
  # respectively.

  push %ebx
  push %ecx

  mov 12(%esp), %ebx
  mov 8(%esp), %ecx

  # Push everything to the stack (including %ebx and %ecx), and change the value of %edx to
  # the given vector number (vNum).

  pushal
  mov \vNum, %edx

  # Call the IsrFaultWithError() function with three arguments - %ebx (the error code from
  # earlier), %ecx (%eip), and %edx (vNum).

  push %ebx
  push %ecx
  push %edx

  call IsrFaultWithError

  pop %edx
  pop %ecx
  pop %ebx

  # Pop everything from the stack (and then pop %ebx and %ecx), and finally, after everything,
  # use the 'iret' instruction to return from our ISR.

  popal
  pop %ebx
  pop %ecx

  iret

.endm


# (macro) IsrAbortStub()
#
# Inputs: \vNum/%edx - The specific vector number of our interrupt.
#         (%eip/ecx - The address of the instruction that caused the exception.)
#
# Outputs: (None; in fact, it doesn't even return)
#
# This macro essentially serves as a template ISR (interrupt service handler) / 'handler' for
# exceptions that are problematic *and* unrecoverable, regardless of whether they provide an
# error code or not.
#
# It takes one explicit argument ('vNum', the vector/interrupt number), and one implicit
# argument (%eip/ecx, the instruction pointer at the time of the exception, which has already
# been pushed onto the stack by our firmware).
#
# After that, it calls IsrAbort(), which is defined in Int.c, and which does not return - in
# fact, it displays an error message and halts the entire system.

.macro IsrAbortStub vNum

  # Push %ecx to the stack (to save the original value of it), and move the instruction
  # pointer of the instruction that caused the exception to %ecx.

  push %ecx
  mov 4(%esp), %ecx

  # Push everything to the stack (including %ecx), and change the value of %edx to the given
  # vector number (vNum).

  pushal
  mov \vNum, %edx

  # Call the IsrAbort() function with two arguments - %ecx (%eip), and %edx (vNum).

  push %ecx
  push %edx

  call IsrAbort

  pop %edx
  pop %ecx

  # The function we just called *really* shouldn't return, but if it somehow does, pop everything
  # from the stack (and then %ecx again), and finally return from our ISR.

  popal
  pop %ecx
  iret

.endm


# (macro) IsrLogStub()
#
# Inputs: \vNum/%edx - The specific vector number of our interrupt.
#
# Outputs: (None)
#
# This macro is a little like the other template ISRs above, but it's more limited. It only
# takes one explicit argument ('vNum', the vector/interrupt number), and it only shows a simple
# information message on the screen (by calling IsrLog(), which is defined in Int.c).
#
# It's intended for interrupts/'exceptions' that aren't really problematic, like NMIs (which
# aren't even considered exceptions).

.macro IsrLogStub vNum

  # Push everything to the stack, and change the value of %edx to the given vector number
  # (vNum).

  pushal
  mov \vNum, %edx

  # Call the IsrLog() function with one argument - %edx (vNum).

  push %edx
  call IsrLog
  pop %edx

  # Pop everything from the stack, and use iret to return from our ISR.

  popal
  iret

.endm


# (macro) IrqStub()
#
# Inputs: \vNum/%edx - The specific IRQ number of our interrupt.
#         \pNum/%ecx - The port we want to read from, if applicable (if not, use $00h).
#
# Outputs: (None)
#
# This macro is also a template ISR, but it's a little different from the other ones, in that
# it's supposed to be used with IRQs.
#
# Because of that, it doesn't take any implicit arguments, and it takes *two* explicit
# arguments instead of one - 'vNum', the IRQ number, and 'pNum', the port we want to read from,
# if applicable (some IRQs, like the PS/2 keyboard IRQ, require this).
#
# Other than that though, it works largely like a regular ISR, and it forwards those two
# arguments to IrqHandler(), which is defined in Irq.c.

.macro IrqStub vNum pNum

  # Push everything to the stack, and change the value of %edx and %ecx to the given vector
  # number (vNum) and port number (pNum) respectively.

  pushal

  mov \vNum, %edx
  mov \pNum, %ecx

  # Call the IrqHandler() with two arguments - %edx (vNum) and %ecx (pNum).

  push %ecx
  push %edx

  call IrqHandler

  pop %edx
  pop %ecx

  # Tell the PIC that we acknowledge its IRQ, by sending the command $20h to the command ports
  # for both PIC-A ($20h) and PIC-B ($A0h).

  mov $0x20, %al
  outb %al, $0x20
  outb %al, $0xA0

  # Pop everything from the stack, and use iret to return from our ISR.

  popal
  iret

.endm


# Our bootloader's ISRs. Each one of these will need to be specified as the offset in an IDT
# entry when we initialize the IDT later on, from $00h to $1Fh.

# For reference, see volume 3 of the Intel(R) 64 and IA-32 Architectures Software Developer’s
# Manual, and volume 2 of the AMD64 Architecture Programmer’s Manual.

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

IsrDoubleFault:
  IsrAbortStub $8

IsrCoprocessorOverrun:
  IsrFaultStub $9

IsrInvalidTss:
  IsrFaultStubWithError $10

IsrSegmentFault:
  IsrFaultStubWithError $11

IsrStackFault:
  IsrFaultStubWithError $12

IsrGpFault:
  IsrFaultStubWithError $13

IsrPageFault:
  IsrFaultStubWithError $14

IsrReservedA:
  IsrLogStub $15

Isr87Fault:
  IsrFaultStub $16

IsrAlignCheck:
  IsrFaultStubWithError $17

IsrMachineCheck:
  IsrAbortStub $18

IsrSimdFault:
  IsrFaultStub $19

IsrVirtFault:
  IsrFaultStub $20

IsrControlFault:
  IsrFaultStubWithError $21

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

IsrHypervisorFault:
  IsrFaultStub $28

IsrVmmFault:
  IsrFaultStubWithError $29

IsrSecurityFault:
  IsrFaultStubWithError $30

IsrReservedH:
  IsrLogStub $31

# Our bootloader's ISRs for IRQs between $20h and $2Fh. Again, for reference, see volume 3 of
# the Intel(R) 64 and IA-32 Architectures Software Developer's Manual.

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
