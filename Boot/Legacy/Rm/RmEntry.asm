; Copyright (C) 2023 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 0C000h] ; We are at C000h.
[BITS 32] ; This is 32-bit code.. for now.

; Start of our program pretty much

; - [Preparations]
; - Prepare a 16-bit real mode 'payload' (ideally at 0D00-0F00h)
; - Prepare a suitable 16-bit real mode GDT and IDT
; - Prepare a suitable 32-bit protected mode GDT
; - Prepare some sort of data area to tell the payload what to do

; - [Process]
; - https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-326.html
; - (Essentially, disable interrupts, go into 16 bit protected mode, switch from protected to real
; - mode, execute your payload, and do the reverse of that)

; ...
; ...

prepareRealMode:

  ; Disable interrupts (except NMIs); as we're switching between different modes, keeping them
  ; on *will* interfere with the process and potentially crash the system.

  cli

  ; Load 16-bit GDT

  lgdt [realModeGdtDescriptor]

  ; Far jump to 16-bit protected mode (setting cs to our code selector, 08h)

  jmp 08h:prepareRealMode16

[BITS 16]

; ...
; ...

prepareRealMode16:

  ; Load ds-ss with our new GDT's data segment.

  mov ax, 10h
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Load real mode IDT

  lidt [realModeIdtDescriptor]

  ; Disable protected mode by clearing the first bit of CR0.

  mov eax, cr0
  and eax, 0FFFFFFFEh
  mov cr0, eax

  ; Far jump to our real mode code

  jmp 00h:0D000h

; -----------------------------------

; 16-bit GDT descriptor

realModeGdtDescriptor:

  dw 24 - 1 ; We have three segments, each of which is 8 bytes long, so our GDT is 24 bytes.
  dd realModeGdt ; The location of our GDT is at the realModeGdt label.

; 16-bit GDT

realModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all 1MiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all 1MiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

; 16-bit IDT/IVT/idfk descriptor

realModeIdtDescriptor:

  dw (400h - 1) ; The real-mode IDT is pretty much always 400h bytes long.
  dd 0 ; It's also usually at the very start of memory (0h).

; -----------------------------------

; This tells our assembler that we want the rest of our program (essentially any areas that haven't
; been used) to be filled with zeroes, up to the 1000h (4096th) byte.

times 1000h-($-$$) db 0
