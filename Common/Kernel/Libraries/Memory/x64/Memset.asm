; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

EXTERN _SimdRegisterArea

GLOBAL _Memset_RepStosb
GLOBAL _Memset_Sse2
GLOBAL _Memset_Avx
GLOBAL _Memset_Avx512f


; This function shouldn't change any preserved registers.
; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

_Memset_RepStosb:

  ; `rep stosb` is an instruction that fills RCX bytes at [RDI] with the
  ; value specified in AL, so let's map our registers accordingly:

  mov rax, rsi
  mov rcx, rdx

  ; At this point, we're almost ready to fill bytes; we just need to
  ; align the buffer address (in RDI) to 16 bytes, if applicable.

  ; (Calculate (Buffer % 16) in R8 - if it's zero, then we already have
  ; an aligned address, otherwise, continue onto .FillUnalignedData)

  mov r8, rdi
  and r8, (16 - 1)

  cmp r8, 0
  je .FillAlignedData

  ; If the buffer address isn't 16-byte-aligned, copy the remainder
  ; (also using `rep stosb`, but *only* for the remainder)

  .FillUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep stosb

    pop rcx
    sub rcx, r8

  ; Now that we know that Buffer is 16-byte-aligned, we can safely
  ; use `rep stosb` to copy everything else, before returning.

  .FillAlignedData:

    cld
    rep stosb

    ret



; This function shouldn't change any preserved registers, other than XMM
; ones, which requires `fxsave` support.

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

_Memset_Sse2:

  ; First, let's check to see if the buffer address is 16-byte-aligned;
  ; and if not, use `rep stosb` to fill it out.)

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (16 - 1)

  mov r8, 16
  sub r8, r9

  ; (Store `Character` (RSI) in RAX)

  mov rax, rsi

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 0
  je .FillAlignedData

  ; If the buffer address isn't 16-byte-aligned, copy the remainder
  ; (also using `rep stosb`, but *only* for the remainder)

  .FillUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep stosb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for SSE.

  .FillAlignedData:

    ; (Save the current state of the SSE registers, using `fxsave`)

    fxsave [_SimdRegisterArea]

    ; (Calculate the amount of 256-byte blocks we'll need to fill out
    ; in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 8

    and rcx, (256 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Move RAX to the lower half of XMM0, and broadcast it to its
    ; higher half as well (using `punpcklqdq`))

    movq xmm0, rax
    punpcklqdq xmm0, xmm0

    ; (Use the `movdqa` instruction to copy XMM0's value to every
    ; other SSE register.)

    movdqa xmm1, xmm0
    movdqa xmm2, xmm0
    movdqa xmm3, xmm0
    movdqa xmm4, xmm0
    movdqa xmm5, xmm0
    movdqa xmm6, xmm0
    movdqa xmm7, xmm0
    movdqa xmm8, xmm0
    movdqa xmm9, xmm0
    movdqa xmm10, xmm0
    movdqa xmm11, xmm0
    movdqa xmm12, xmm0
    movdqa xmm13, xmm0
    movdqa xmm14, xmm0
    movdqa xmm15, xmm0

  ; Fill each 256-byte block with SSE registers, using R9 as a
  ; counter (R9 == (Size / 256)).

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `movdqa` instruction to fill the next 256 bytes,
    ; while keeping XMM0 ~ XMM15 in cache (this is a temporal move))

    movdqa [rdi+0], xmm0
    movdqa [rdi+16], xmm1
    movdqa [rdi+32], xmm2
    movdqa [rdi+48], xmm3
    movdqa [rdi+64], xmm4
    movdqa [rdi+80], xmm5
    movdqa [rdi+96], xmm6
    movdqa [rdi+112], xmm7
    movdqa [rdi+128], xmm8
    movdqa [rdi+144], xmm9
    movdqa [rdi+160], xmm10
    movdqa [rdi+176], xmm11
    movdqa [rdi+192], xmm12
    movdqa [rdi+208], xmm13
    movdqa [rdi+224], xmm14
    movdqa [rdi+240], xmm15

    ; (Move to the next 256-byte block, and repeat the loop)

    add rdi, 256
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Restore the previous state of the SSE registers, and return.

  .Cleanup:

    fxrstor [_SimdRegisterArea]
    ret



; This function shouldn't change any preserved registers, other than YMM
; ones, which requires `xsave` support.

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

_Memset_Avx:
  jmp _Memset_Sse2



; This function shouldn't change any preserved registers, other than ZMM
; ones, which requires `xsave` support.

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

_Memset_Avx512f:
  jmp _Memset_Avx
