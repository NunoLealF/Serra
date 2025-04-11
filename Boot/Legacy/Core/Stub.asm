; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 32]

SECTION .text
GLOBAL LongmodeStub

; It's assumed that this stub will behave as a regular 32-bit function
; would, so we need to initialize a new call frame, and get arguments:

; (uintptr InfoTable [ebx], uintptr Pml4 [edx])
; (We discard [eax] and [ecx])

LongmodeStub:

  ; Deal with the C calling convention / initialize the call frame.

  push ebp
  mov ebp, esp

  mov ebx, [ebp + 8]
  mov edx, [ebp + 12]

  ; Set up the necessary environment - disable interrupts, etc.

  cli

  ; Enable page address extensions (PAE), by setting bit 5 of CR4.

  push eax

  mov eax, cr4
  or eax, (1 << 5)
  mov cr4, eax

  pop eax

  ; Store the address of our PML4 in the CR3 register (making sure to
  ; exclude bits 0-11).

  mov eax, cr3
  and edx, 0xFFFFF000
  or eax, edx
  mov cr3, eax

  ; Get the current value of the EFER register, and set bit 8 (long mode)

  mov ecx, 0xC0000080
  rdmsr

  or eax, (1 << 8)
  wrmsr

  ; Enable (long mode) paging by setting bit 31 of CR0.

  mov eax, cr0
  or eax, (1 << 31)
  mov cr0, eax

  ; At this point, we're finally in compatibility (IA-32e) mode, but
  ; in order to start executing 64-bit code, we actually need to
  ; update the GDT with 64-bit segments, so let's do that.

  lgdt [longModeGdtDescriptor]

  ; Now that we've taken care of that, we can *finally* load the
  ; new GDT, and actually enter long mode!

  jmp 0x08:JumpToHigherHalf


; -----------------------------------

[BITS 64]

JumpToHigherHalf:

  ; Our CPU has successfully entered 64-bit mode, hooray! <3

  ; For now, let's just set up the stack and segment registers
  ; before jumping to the actual entrypoint.

  mov ax, 0x10

  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Now, we can finally jump to the entrypoint:

  ;mov rsp, (0xFFFFFF8000000000 - 8)  (*we don't have a stack mapped yet!!!*)
  ;mov rdi, rbx                       (*this^, but also, attribute noreturn = unnecessary)
  ;push rbp

  call 0xFFFFFF8000000000


; -----------------------------------

; Our 64-bit GDT.

longModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all of memory *no matter what*)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 10101111b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all of memory *no matter what*)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

; Our 64-bit GDT descriptor.

longModeGdtDescriptor:

  .Size: dw (longModeGdtDescriptor - longModeGdt - 1) ; sizeof(longModeGdt) - 1
  .Address: dq longModeGdt ; The location of longModeGdt
