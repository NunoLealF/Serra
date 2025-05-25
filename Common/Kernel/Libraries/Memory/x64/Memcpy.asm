; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

EXTERN _SimdRegisterArea

GLOBAL _Memcpy_RepMovsb
GLOBAL _Memcpy_Sse2
GLOBAL _Memcpy_Avx
GLOBAL _Memcpy_Avx512f


; TODO - In theory this shouldn't alter any preserved registers.
; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

_Memcpy_RepMovsb:

  ; `rep movsb` is an instruction that copies RCX bytes from [RSI] to
  ; [RDI], so let's map our registers accordingly:

  mov rcx, rdx

  ; At this point, we're almost ready to fill bytes; we just need to align
  ; the destination address (in RDI) to 16 bytes, if possible.

  ; (Calculate (Destination % 16) in R8 - if it's zero, then we already have
  ; an aligned address, but otherwise, continue onto .MoveUnalignedData)

  mov r8, rdi
  and r8, (16 - 1)

  cmp r8, 0
  je .MoveAlignedData

  ; If the destination address isn't 16-byte-aligned, copy the remainder
  ; (also using `rep movsb` (and *only* the remainder))

  .MoveUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep movsb

    pop rcx
    sub rcx, r8

  ; Now that we know that Destination is 16-byte-aligned, we can safely
  ; use `rep movsb` to copy everything else, before returning.

  .MoveAlignedData:

    cld
    rep movsb

    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push XMM(n) still.

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

_Memcpy_Sse2:

  ; First, let's check to see if the destination addresses is 16-byte-
  ; -aligned, and if not, use `rep movsb` to copy the remainder.

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (16 - 1)

  mov r8, 16
  sub r8, r9

  ; (Store `Size` (RDX) in RCX, since it's used for `rep movsb`)

  mov rcx, rdx

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 16
  je .MoveAlignedData

  ; If the destination address isn't 16-byte-aligned, copy the remainder
  ; (also using `rep movsb`, but *only* for the remainder)

  .MoveUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep movsb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for SSE.

  .MoveAlignedData:

    ; (Save the current state of the SSE registers, using `fxsave`)

    fxsave [_SimdRegisterArea]

    ; (Calculate the number of 256-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 8

  ; Move each 256-byte block using SSE registers, using R9 as a counter,
  ; and the `movdqu` and `movntdq` instructions

  .MoveBlockData:

    ; (The number of 256-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read sixteen unaligned double quadwords (16-byte blocks) into each
    ; XMM register, using the `movdqu` instruction - keep in mind that
    ; we're reading from [RSI+n], which is the same as (*Source + n))

    movdqu xmm0, [rsi+0]
    movdqu xmm1, [rsi+16]
    movdqu xmm2, [rsi+32]
    movdqu xmm3, [rsi+48]
    movdqu xmm4, [rsi+64]
    movdqu xmm5, [rsi+80]
    movdqu xmm6, [rsi+96]
    movdqu xmm7, [rsi+112]
    movdqu xmm8, [rsi+128]
    movdqu xmm9, [rsi+144]
    movdqu xmm10, [rsi+160]
    movdqu xmm11, [rsi+176]
    movdqu xmm12, [rsi+192]
    movdqu xmm13, [rsi+208]
    movdqu xmm14, [rsi+224]
    movdqu xmm15, [rsi+240]

    ; (Write those values back to memory, using the `movntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    movntdq [rdi+0], xmm0
    movntdq [rdi+16], xmm1
    movntdq [rdi+32], xmm2
    movntdq [rdi+48], xmm3
    movntdq [rdi+64], xmm4
    movntdq [rdi+80], xmm5
    movntdq [rdi+96], xmm6
    movntdq [rdi+112], xmm7
    movntdq [rdi+128], xmm8
    movntdq [rdi+144], xmm9
    movntdq [rdi+160], xmm10
    movntdq [rdi+176], xmm11
    movntdq [rdi+192], xmm12
    movntdq [rdi+208], xmm13
    movntdq [rdi+224], xmm14
    movntdq [rdi+240], xmm15

    ; (Unlike `rep movs*`, these instructions don't automatically increment
    ; or decrement registers for us, so we have to do that ourselves)

    add rsi, 256
    add rdi, 256

    sub rcx, 256

    ; (Repeat the loop, decrementing our counter (R9))

    dec r9
    jmp .MoveBlockData

  ; Move the remainder of the data, using `rep movsb` again.

  .MoveRemainder:

    cld
    rep movsb

  ; Restore the previous state of the SSE registers, and return.

  .Cleanup:

    fxrstor [_SimdRegisterArea]
    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push YMM(n) still.

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

_Memcpy_Avx:
  jmp _Memcpy_Sse2



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push ZMM(n) still.

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

_Memcpy_Avx512f:
  jmp _Memcpy_Avx
