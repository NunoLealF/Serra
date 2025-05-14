; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 32]

SECTION .text

EXTERN KernelEntrypoint
EXTERN KernelStack
GLOBAL TransitionStub

; It's assumed that this stub will behave as a regular 32-bit function
; would, so we need to initialize a new call frame, and get arguments:

; (kernelInfoTable* InfoTable [ebp+8], void* Pml4 [ebp+12])
; (We discard [eax] and [edx])

TransitionStub:

  ; Deal with the C calling convention / initialize the call frame.

  push ebp
  mov ebp, esp

  mov edi, [ebp + 8]
  mov esi, [ebp + 12]

  ; Set up the necessary environment - disable interrupts, and push
  ; everything to the stack.

  pushad
  cli

  ; Store our current stack pointer.

  mov [SaveStack], esp

  ; Enable page address extensions (PAE), by setting bit 5 of CR4.

  mov eax, cr4
  or eax, (1 << 5)
  mov cr4, eax

  ; Store the address of our PML4 in the CR3 register (making sure to
  ; *clear* bits 0-11; preserving them can cause it to crash).

  and esi, 0xFFFFF000
  mov cr3, esi

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
  ; update the GDT with 64-bit segments, so let's do that:

  lgdt [LongModeGdtDescriptor]

  ; Now that we've taken care of that, we can *finally* load the
  ; new GDT, and actually enter long mode!

  jmp 0x08:JumpToKernel


; ------------------------------------------------------------

[BITS 64]

JumpToKernel:

  ; Our CPU has successfully entered 64-bit mode, hooray! <3

  ; For now, let's just set up the stack and segment registers
  ; before jumping to the actual entrypoint.

  mov ax, 0x10

  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; (Set up the stack)

  mov rsp, [KernelStack]
  sub rsp, 128

  push rbp

  ; Now, we can finally jump to the entrypoint:

  mov rcx, [KernelEntrypoint]
  call rcx

  ; If we're here, then that means the entrypoint returned,
  ; so we need to transfer back control.

  ; (Turn rax into edx:eax, since that's how you're supposed
  ; to return 64-bit values in 32-bit functions)

  mov rdx, rax
  shr rdx, 32

  mov [SaveEax], eax
  mov [SaveEdx], edx

  ; The first step in that process is to switch back to
  ; compatibility (IA-32e) mode, by loading a valid 32-bit
  ; GDT, like this:

  lgdt [ProtectedModeGdtDescriptor]

  ; Next, we need to set CS to the code segment (08h), just
  ; as we did before switching into long mode (though this
  ; requires a far *return*, not jump):

  push qword 0x08
  push qword ReturnFromKernel
  retfq


; ------------------------------------------------------------

[BITS 32]

ReturnFromKernel:

  ; Now that we're back in 32-bit mode, we just need to unwind
  ; the changes we made to get here.

  ; First, let's set up the segment registers (again):

  mov ax, 0x10

  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Second, let's disable (long mode) paging, by clearing bit
  ; 31 of CR0:

  mov eax, cr0
  and eax, ~(1 << 31)
  mov cr0, eax

  ; Get the current value of the EFER register, and clear bit 8
  ; (long mode, or the 'LME' bit) - we want to preserve eax, so
  ; we'll temporarily push everything to the stack.

  pushad

  mov ecx, 0xC0000080
  rdmsr

  and eax, ~(1 << 8)
  wrmsr

  popad

  ; We don't really need to deal with CR3 or disable PAE, since
  ; those steps don't affect anything in protected mode, so we
  ; can restore the previous program state and return.

  mov esp, [SaveStack]

  ; Pop registers - we use SaveEaxEdx to temporarily store eax
  ; and edx, since they contain our return value.

  popad

  mov edx, [SaveEdx]
  mov eax, [SaveEax]

  ; Pop the base pointer, and return

  pop ebp
  ret


; ------------------------------------------------------------

; A space for some of our original 32-bit registers, if we
; ever need to preserve/save/restore them.

SaveEax:
  dd 0

SaveEdx:
  dd 0

SaveStack:
  dd 0


; ------------------------------------------------------------

; Our 32-bit protected mode GDT.

ProtectedModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all 4GiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all 4GiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

; Our 32-bit protected mode GDT descriptor.

ProtectedModeGdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd ProtectedModeGdt ; The location of our GDT is the ProtectedModeGdt label.


; ------------------------------------------------------------

; Our 64-bit GDT.

LongModeGdt:

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

LongModeGdtDescriptor:

  .Size: dw (LongModeGdtDescriptor - LongModeGdt - 1) ; sizeof(LongModeGdt) - 1
  .Address: dq LongModeGdt ; The location of LongModeGdt
