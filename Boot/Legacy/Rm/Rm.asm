; Copyright (C) 2023 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 0D000h] ; We are at 7C00h.
[BITS 16] ; This is 16-bit code.

; Just in case

cli

; Reset segment registers to 0 (for some reason real mode's like that)

mov ax, 0
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

mov sp, 7C00h
; we really need to find somewhere where we can save the stack pointer/segment
; like, push and pop doesn't work because it relies on the stack lol

; (btw, if this doesn't work - a possible candidate is that the GDT might be wrong)
; (like, it might be growing downwards instead of upwards, that sorta thing)

; Okay, test

sti
mov ah, 0
mov al, 13h
int 10h

mov ah, 0Ch
mov al, 0Bh
mov bh, 0
mov cx, 0
mov dx, 0
int 10h

loopuwu:

cli
hlt
jmp loopuwu

; -----------------------------------

; This tells our assembler that we want the rest of our program (essentially any areas that haven't
; been used) to be filled with zeroes, up to the 1000h (4096th) byte.

times 1000h-($-$$) db 0
