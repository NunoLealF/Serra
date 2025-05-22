; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

EXTERN _SimdRegisterArea
GLOBAL _MemcpyBase
GLOBAL _MemcpySse2

; TODO - In theory this shouldn't even alter any preserved registers.
; (void* Source (RSI), const void* Destination (RDI), uint64 Size (RDX))

_MemcpyBase:

  ; (Calculate the number of 8-byte blocks we need to move (in RCX), as well
  ; as the remainder (in RDX))

  mov rcx, rdx
  shr rcx, 3

  and rdx, 7

  ; (Clear the direction flag, just in case)

  cld

  ; (Move 8-byte blocks from Source to Destination, using `rep movsq`,
  ; and then move the remainder using `rep movsb`

  rep movsq

  mov rcx, rdx
  rep movsb

  ; (Return.)
  
  ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push XMM(n) still.

; (void* Source (RSI), const void* Destination (RDI), uint64 Size (RDX))

_MemcpySse2:

  .BeforeLoop:

    ; (Save the current state of the SSE registers, using `fxsave`)

    fxsave [_SimdRegisterArea]

    ; (Deal with non-16-byte aligned addresses - we do this by calculating
    ; the remainder in RCX, and using rep movsb)

    mov rcx, rdx
    and rcx, 15

    sub rdx, rcx

    rep movsb

    ; (Calculate the number of 256-byte 'blocks' we need to move in R8)

    mov r8, rdx
    shr r8, 8

  .Loop:

    ; (The number of 256-byte blocks left to copy is stored in R8, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r8, 0
    je .AfterLoop

    ; (Read sixteen aligned double quadwords (16-byte blocks) into each XMM
    ; register, using the `movdqa` instruction - keep in mind that we're
    ; reading from [RSI+n], which is the same as (*Source + n))

    movdqa xmm0, [rsi+0]
    movdqa xmm1, [rsi+16]
    movdqa xmm2, [rsi+32]
    movdqa xmm3, [rsi+48]
    movdqa xmm4, [rsi+64]
    movdqa xmm5, [rsi+80]
    movdqa xmm6, [rsi+96]
    movdqa xmm7, [rsi+112]
    movdqa xmm8, [rsi+128]
    movdqa xmm9, [rsi+144]
    movdqa xmm10, [rsi+160]
    movdqa xmm11, [rsi+176]
    movdqa xmm12, [rsi+192]
    movdqa xmm13, [rsi+208]
    movdqa xmm14, [rsi+224]
    movdqa xmm15, [rsi+240]

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

    ; (Repeat the loop, decrementing our counter (R8))

    dec r8
    jmp .Loop

  .AfterLoop:

    ; (Unless our data was 256-byte aligned already, we most likely still
    ; have some data left to copy, so let's use `rep movsq` for that.)

    mov rcx, rdx

    and rcx, 255
    shr rcx, 3

    rep movsq

    ; (Restore SSE registers, using `fxrstor`)

    fxrstor [_SimdRegisterArea]

    ; (Return.)

    ret
