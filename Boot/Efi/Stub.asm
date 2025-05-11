; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

SECTION .text

EXTERN KernelEntrypoint
EXTERN KernelStack
GLOBAL TransitionStub

; It's assumed that this stub will behave as a regular 64-bit function (with
; the EFI/Microsoft ABI) would, so we need to initialize a new call frame,
; and get arguments:

; (kernelInfoTable* InfoTable [rcx], void* KernelEntrypoint [rdx], void* KernelStack [r8])
; (We preserve rbx, rdi, rsi, rsp, rbp, r12, r13, r14 and r15)

TransitionStub:

  ; Push all preserved registers.

  push rbx
  push rdi
  push rsi
  push rsp
  push rbp
  push r12
  push r13
  push r14
  push r15

  ; Save the current stack pointer, so we can restore it later.
  ; (In this case, we store rsp in r15)

  mov r15, rsp

  ; Set the stack pointer to the one that will be used by the kernel.

  mov rsp, r8
  sub rsp, 128

  ; Initialize rbp and the call frame.

  push rbp
  mov rbp, rsp

  ; Call the kernel, this time using the regular (System-V) ABI, which
  ; makes us pass the first argument (InfoTable) in rdi:

  mov rdi, rcx
  mov rax, rdx
  call rax

  ; Once it returns, we can gracefully transfer control back to the EFI
  ; application. First, we need to restore the old stack pointer:

  mov rsp, r15

  ; After that, we can pop all of the preserved registers back from the
  ; stack, and return.

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbp
  pop rsp
  pop rsi
  pop rdi
  pop rbx

  ret
