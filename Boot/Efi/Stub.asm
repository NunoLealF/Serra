; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

SECTION .text

EXTERN KernelEntrypoint
EXTERN KernelStackTop
GLOBAL TransitionStub

; It's assumed that this stub will behave as a regular 64-bit function (with
; the EFI/Microsoft ABI) would, so we need to initialize a new call frame,
; and get arguments:

; (commonInfoTable* InfoTable [rcx], void* KernelEntrypoint [rdx], void* KernelStackTop [r8])
; (We preserve RBX, RDI, RSI, RSP, RBP, R12, R13, R14 and R15.)
; (We discard RCX, RDX and R8; and return in RAX.)

TransitionStub:

  ; (1) Push all preserved registers.

  push rbp
  mov rbp, rsp

  push rbx
  push rdi
  push rsi
  push rsp
  push rbp
  push r12
  push r13
  push r14
  push r15

  ; (2) Disable interrupts - we can reenable them later, though.

  cli

  ; (3) Store the kernel entrypoint in RBX, and save the current stack
  ; pointer in R15 - both of these are preserved by the ABI.

  mov rbx, rdx
  mov r15, rsp

  ; (4) Set the stack pointer to the one that will be used by the
  ; kernel (we subtract 128 bytes, just in case).

  mov rsp, r8
  sub rsp, 128

  ; (5) Set up the call frame (the stack needs to be 16-byte
  ; aligned, so we subtract 8 bytes before pushing rbp).

  sub rsp, 8
  push rbp

  mov rbp, rsp

  ; (6) Call the kernel, this time using the regular (System-V) ABI,
  ; which dictates we should pass the first argument (InfoTable) in RDI.

  mov rdi, rcx
  call rbx ; (KernelEntrypoint -> rdx -> rbx)

ReturnToEfiApplication:

  ; Once it returns, we can gracefully transfer control back to the
  ; EFI application.

  ; (1) First, we need to disable interrupts again, in case they've
  ; been enabled:

  cli

  ; (2) Second, we need to restore the old base base and stack
  ; pointers (RBP and RSP, respectively):

  pop rbp
  mov rsp, r15

  ; (3) Next, we can pop all of the preserved registers back from the
  ; stack, before returning.

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbp
  pop rsp
  pop rsi
  pop rdi
  pop rbx

  pop rbp

  ret
