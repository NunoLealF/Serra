; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

SECTION .pmstub
[BITS 32]

; It's assumed that this stub will behave as a regular 32-bit function
; would, so we need to initialize a new call frame, and get arguments:

; (uintptr* InfoTable [ebx], uintptr* Pml4 [edx])
; (We discard [eax] and [ecx])

Stub:

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

  ; Get the current value of the EFER register, and set bit 8 (long mode)
  ; (We can safely discard [edx] as we already stored it in [cr3])

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

  ; We do quickly run into a problem though: *this stub is meant to
  ; be position-independent*, so we can't just hardcode an address
  ; into the GDT descriptor.

  ; However, since we know this stub is guaranteed to be page-
  ; -aligned and fit into one 4KiB page, we *are* able to:

  ; -> Get the value of EIP by using 'call GetCurrentAddressInEdx'
  ; -> Use that as a reference for the current memory position

  call GetCurrentAddressInEdx

  and edx, 0xFFFFF000
  mov ecx, edx
  mov eax, edx

  push eax

  add eax, (JumpToHigherHalf - Stub)
  add ecx, (longModeGdt - Stub)
  add edx, (longModeGdtDescriptor.addressLow - Stub)

  mov [edx], ecx

  ; (Prepare the far jump ahead, we need to load .Address)

  pop ecx
  add ecx, (emulatedFarJump.Address - Stub)

  mov [ecx], eax

  ; Now that we've taken care of that, we can *finally* load the
  ; new GDT, and actually enter long mode!

  ; We can't directly jump to our higher half kernel yet, as this
  ; last jump is still in compatibility mode (and therefore
  ; limited to 32-bit), but we can work around it still:

  lgdt [edx - 8]

  ; We *do* need to use the same hack we used in RmWrapper.asm
  ; and hardcode the opcode for a far jump here, but we don't
  ; have much of a choice with position-independent code

  ; (For reference, this is the equivalent of jmp 0x08:Address)

  emulatedFarJump.Opcode: db 0xEA
  emulatedFarJump.Address: dd 0
  emulatedFarJump.Segment: dw 0x08


; -----------------------------------

; Since 'call' pushes eip to the stack, this function basically
; does the equivalent of "mov edx, eip"; pretty neat, uwu

GetCurrentAddressInEdx:

  mov edx, [esp]
  ret


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

  ;mov rsp, (0xFFFFFFFF80000000 - 8)  (*we don't have a stack mapped yet!!!*)
  ;mov rdi, rbx                       (*this^, but also, attribute noreturn = unnecessary)
  ;push rbp

  jmp 0xFFFFFFFF80000000

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

  .addressHigh: dd 0 ; This is guaranteed to be 0 for any 32-bit address
  .addressLow: dd 0 ; (This will later be set to the location of longModeGdt)


; -----------------------------------

; Tell our assembler that we want to pad the rest of our stub up to
; the 4096th (or 0x1000th) byte, so it can be page-aligned.

times 0x1000 - ($-$$) db 0
