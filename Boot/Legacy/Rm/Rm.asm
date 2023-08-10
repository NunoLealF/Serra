; Copyright (C) 2023 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 0C000h] ; We are at C000h.
[BITS 32] ; This is 32-bit code.. for now.

; Start of our program pretty much

extern realModeRegisters

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

prepareRealMode32:

  ; Disable interrupts (except NMIs); as we're switching between different modes, keeping them
  ; on *will* interfere with the process and potentially crash the system.

  cli

  ; Save ESP (stack pointer)

  mov [saveStack], esp

  ; Load 16-bit GDT

  lgdt [realModeGdtDescriptor]

  ; Far jump to 16-bit protected mode (setting cs to our code selector, 08h)

  jmp 08h:prepareRealMode16


; ...
; ...

[BITS 16]

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

  jmp 00h:initRealMode


; ...
; ...

initRealMode:

  ; Just in case

  cli

  ; Reset segment registers to 0 (for some reason real mode's like that)

  mov ax, 0
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Set stack to 7C00h

  mov sp, 7C00h


; ...
; ...

setupRealMode:

  ; Import data from the realModeRegisters struct at CE00h

  mov bp, 0CE00h

  mov eax, [bp]
  mov ebx, [bp+4]
  mov ecx, [bp+8]
  mov edx, [bp+12]
  mov si, [bp+16]
  mov di, [bp+18]
  mov bp, [bp+20]


  ; Test (this should display a <3 on the screen)

  sti

  ;mov ah, 0Eh
  ;mov al, '<'
  mov bh, 0
  mov bl, 07h

  ; Test
  mov cl, 'R'
  mov ch, 'm'

  int 10h

  ; Export data back to the realModeRegisters struct at CE00h

  push bp

  mov bp, 0CE00h
  mov [bp], eax
  mov [bp+4], ebx
  mov [bp+8], ecx
  mov [bp+12], edx
  mov [bp+16], si
  mov [bp+18], di

  pop bp

  mov si, 0CE00h
  mov [si+20], bp

  pushfd
  pop eax
  mov [si+22], eax

  ; Go back to protected mode

  jmp prepareProtectedMode16

; ...
; ...

prepareProtectedMode16:

  ; Disable interrupts

  cli

  ; Load protected mode GDT

  lgdt [protectedModeGdtDescriptor]

  ; Enable protected mode

  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; Far jump

  jmp 08h:prepareProtectedMode32


; ...
; ...

[BITS 32]

prepareProtectedMode32:

  ; We're now in protected mode
  ; Restore segment vars

  mov eax, 10h
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax

  ; Restore ESP

  mov esp, [saveStack]

  ; Return (because of the C calling convention, everything else should be saved)

  ret


; -----------------------------------

; This is where we'll reserve our stack pointer - although we save the state of every other
; register with the C calling conventions, we'll still need to know the stack pointer so we can
; actually pop those registers from the stack.

saveStack: dd 0

; -----------------------------------

; 16-bit real and protected mode GDT descriptor.

realModeGdtDescriptor:

  dw 24 - 1 ; We have three segments, each of which is 8 bytes long, so our GDT is 24 bytes.
  dd realModeGdt ; The location of our GDT is at the realModeGdt label.

; 16-bit real and protected mode GDT.

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

; 16-bit real mode IDT descriptor.

realModeIdtDescriptor:

  dw (400h - 1) ; The real-mode IDT is pretty much always 400h bytes long.
  dd 0 ; It's also usually at the very start of memory (0h).

; -----------------------------------

; 32-bit protected mode GDT descriptor.

protectedModeGdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd protectedModeGdt ; The location of our GDT is the protectedModeGdt label.

; 32-bit protected mode GDT.

protectedModeGdt:

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

; This tells our assembler that we want the rest of our program (essentially any areas that haven't
; been used) to be filled with zeroes, up to the E00h (3584th) byte.

times 0E00h-($-$$) db 0
