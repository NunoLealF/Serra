; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

SECTION .pmstub
[BITS 32]

; It's assumed that this stub will behave as a regular 32-bit function
; would, so we need to initialize a new call frame, and get arguments:

; (uintptr* InfoTable [ecx], uintptr* Pml4 [edx])

Stub:

  ; (Deal with the C calling convention / initialize the call frame.)

  push ebp
  mov ebp, esp

  mov ecx, [ebp + 8]
  mov edx, [ebp + 12]

  ; (do something bla)

  pop ebp
  ret


; -----------------------------------

; 64-bit GDT descriptor.

longModeGdtDescriptor:

  dw 1234
  dd longModeGdt ; The location of our GDT is the longModeGdt label.

; 64-bit GDT.

longModeGdt:

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


; -----------------------------------

; Tell our assembler that we want to pad the rest of our stub up to the
; 4096th (0x1000th) byte, so it can be page-aligned.

times 0x1000 - ($-$$) db 0
