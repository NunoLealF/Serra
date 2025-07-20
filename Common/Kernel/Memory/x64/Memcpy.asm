; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

GLOBAL Memcpy_RepMovsb
GLOBAL Memcpy_Sse2
GLOBAL Memcpy_Avx
GLOBAL Memcpy_Avx512f


; TODO - In theory this shouldn't alter any preserved registers.
; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

Memcpy_RepMovsb:

  ; `rep movsb` is an instruction that copies RCX bytes from [RSI] to
  ; [RDI], so let's map our registers accordingly:

  mov rcx, rdx

  ; If our size is less than 16 bytes, then not only is there little
  ; benefit to aligning to 16-byte boundaries, but we can also crash
  ; the system if Size > (Buffer % 16).

  cmp rdx, 16
  jl .MoveAlignedData

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

    sfence
    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push XMM(n) still. (Size >= 256)

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

Memcpy_Sse2:

  ; First, let's check if the destination addresses is 16-byte-
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

    ; (Calculate the number of 64-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 6

  ; Move each 64-byte block using SSE registers, using R9 as a counter,
  ; and the `movdqu` and `movntdq` instructions

  .MoveBlockData:

    ; (The number of 64-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read four aligned double quadwords (16-byte blocks) into the
    ; XMM registers, using the `movdqu` instruction - keep in mind that
    ; we're reading from [RSI+n], which is the same as (*Source + n))

    prefetchnta [rsi+128]

    movdqu xmm0, [rsi+0]
    movdqu xmm1, [rsi+16]
    movdqu xmm2, [rsi+32]
    movdqu xmm3, [rsi+48]

    ; (Write those values back to memory, using the `movntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    movntdq [rdi+0], xmm0
    movntdq [rdi+16], xmm1
    movntdq [rdi+32], xmm2
    movntdq [rdi+48], xmm3

    ; (Unlike `rep movs*`, these instructions don't automatically increment
    ; or decrement registers for us, so we have to do that ourselves)

    add rsi, 64
    add rdi, 64

    sub rcx, 64

    ; (Repeat the loop, decrementing our counter (R9))

    dec r9
    jmp .MoveBlockData

  ; Move the remainder of the data, using `rep movsb` again.

  .MoveRemainder:

    cld
    rep movsb

  ; Now that we're done, we can safely return.

  .Cleanup:

    sfence
    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push YMM(n) still. (Size >= 512)

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

Memcpy_Avx:

  ; First, let's check if the destination address is 32-byte-aligned,
  ; and if not, use `rep movsb` to copy the remainder.

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (32 - 1)

  mov r8, 32
  sub r8, r9

  ; (Store `Size` (RDX) in RCX, since it's used for `rep movsb`)

  mov rcx, rdx

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 32
  je .MoveAlignedData

  ; If the destination address isn't 32-byte-aligned, copy the remainder
  ; (also using `rep movsb`, but *only* for the remainder)

  .MoveUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep movsb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for AVX.

  .MoveAlignedData:

    ; (Calculate the number of 128-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 7

  ; Move each 128-byte block using AVX registers, using R9 as a counter,
  ; and the `vmovdqu` and `vmovntdq` instructions

  .MoveBlockData:

    ; (The number of 128-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read four aligned 32-byte blocks into the YMM registers,
    ; using the `vmovdqu` instruction - keep in mind that we're reading
    ; from [RSI]+n, which is the same as (*Source + n))

    prefetchnta [rsi+256]

    vmovdqu ymm0, [rsi+0]
    vmovdqu ymm1, [rsi+32]
    vmovdqu ymm2, [rsi+64]
    vmovdqu ymm3, [rsi+96]

    ; (Write those values back to memory, using the `vmovntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    vmovntdq [rdi+0], ymm0
    vmovntdq [rdi+32], ymm1
    vmovntdq [rdi+64], ymm2
    vmovntdq [rdi+96], ymm3

    ; (Unlike `rep movs*`, these instructions don't automatically increment
    ; or decrement registers for us, so we have to do that ourselves)

    add rsi, 128
    add rdi, 128

    sub rcx, 128

    ; (Repeat the loop, decrementing our counter (R9))

    dec r9
    jmp .MoveBlockData

  ; Move the remainder of the data, using `rep movsb` again.

  .MoveRemainder:

    cld
    rep movsb

  ; Now that we're done, let's return.

  .Cleanup:

    sfence
    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push ZMM(n) still. (Size >= 2048)

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

Memcpy_Avx512f:

  ; First, let's check if the destination address is 64-byte-aligned,
  ; and if not, use `rep movsb` to copy the remainder.

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (64 - 1)

  mov r8, 64
  sub r8, r9

  ; (Store `Size` (RDX) in RCX, since it's used for `rep movsb`)

  mov rcx, rdx

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 64
  je .MoveAlignedData

  ; If the destination address isn't 64-byte-aligned, copy the remainder
  ; (also using `rep movsb`, but *only* for the remainder)

  .MoveUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep movsb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for AVX-512.

  .MoveAlignedData:

    ; (Calculate the number of 256-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 8

  ; Move each 256-byte block using AVX-512 registers, using R9 as
  ; a counter, and the `vmovdqu` and `vmovntdq` instructions

  .MoveBlockData:

    ; (The number of 256-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read four aligned 64-byte blocks into the ZMM registers, using
    ; the `vmovdqu64` instruction - keep in mind that we're reading
    ; from [RSI]+n, which is the same as (*Source + n))

    prefetchnta [rsi+512]

    vmovdqu64 zmm0, [rsi+0]
    vmovdqu64 zmm1, [rsi+64]
    vmovdqu64 zmm2, [rsi+128]
    vmovdqu64 zmm3, [rsi+192]

    ; (Write those values back to memory, using the `vmovntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    vmovntdq [rdi+0], zmm0
    vmovntdq [rdi+64], zmm1
    vmovntdq [rdi+128], zmm2
    vmovntdq [rdi+192], zmm3

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

  ; Now that we're done, return.

  .Cleanup:

    sfence
    ret
