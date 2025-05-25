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

    ; (Read sixteen unaligned double quadwords (16-byte blocks) into the
    ; XMM registers, using the `movdqu` instruction - keep in mind that
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

    ; (Save the current state of the AVX registers, using `xsave`)

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xsave [_SimdRegisterArea]

    pop rdx
    pop rax

    ; (Calculate the number of 512-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 9

  ; Move each 512-byte block using AVX registers, using R9 as a counter,
  ; and the `vmovdqu` and `vmovntdq` instructions

  .MoveBlockData:

    ; (The number of 512-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read sixteen unaligned 32-byte blocks into the YMM registers,
    ; using the `vmovdqu` instruction - keep in mind that we're reading
    ; from [RSI]+n, which is the same as (*Source + n))

    vmovdqu ymm0, [rsi+0]
    vmovdqu ymm1, [rsi+32]
    vmovdqu ymm2, [rsi+64]
    vmovdqu ymm3, [rsi+96]
    vmovdqu ymm4, [rsi+128]
    vmovdqu ymm5, [rsi+160]
    vmovdqu ymm6, [rsi+192]
    vmovdqu ymm7, [rsi+224]
    vmovdqu ymm8, [rsi+256]
    vmovdqu ymm9, [rsi+288]
    vmovdqu ymm10, [rsi+320]
    vmovdqu ymm11, [rsi+352]
    vmovdqu ymm12, [rsi+384]
    vmovdqu ymm13, [rsi+416]
    vmovdqu ymm14, [rsi+448]
    vmovdqu ymm15, [rsi+480]

    ; (Write those values back to memory, using the `vmovntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    vmovntdq [rdi+0], ymm0
    vmovntdq [rdi+32], ymm1
    vmovntdq [rdi+64], ymm2
    vmovntdq [rdi+96], ymm3
    vmovntdq [rdi+128], ymm4
    vmovntdq [rdi+160], ymm5
    vmovntdq [rdi+192], ymm6
    vmovntdq [rdi+224], ymm7
    vmovntdq [rdi+256], ymm8
    vmovntdq [rdi+288], ymm9
    vmovntdq [rdi+320], ymm10
    vmovntdq [rdi+352], ymm11
    vmovntdq [rdi+384], ymm12
    vmovntdq [rdi+416], ymm13
    vmovntdq [rdi+448], ymm14
    vmovntdq [rdi+480], ymm15

    ; (Unlike `rep movs*`, these instructions don't automatically increment
    ; or decrement registers for us, so we have to do that ourselves)

    add rsi, 512
    add rdi, 512

    sub rcx, 512

    ; (Repeat the loop, decrementing our counter (R9))

    dec r9
    jmp .MoveBlockData

  ; Move the remainder of the data, using `rep movsb` again.

  .MoveRemainder:

    cld
    rep movsb

  ; Restore the previous state of the AVX registers, and return.

  .Cleanup:

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xrstor [_SimdRegisterArea]

    pop rdx
    pop rax

    ret



; TODO - In theory this shouldn't alter any non-SIMD preserved registers,
; but we need to push ZMM(n) still.

; (void* Destination (RDI), const void* Source (RSI), uint64 Size (RDX))

_Memcpy_Avx512f:

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

    ; (Save the current state of the AVX registers, using `xsave`)

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xsave [_SimdRegisterArea]

    pop rdx
    pop rax

    ; (Calculate the number of 2048-byte 'blocks' we need to move in R9)

    mov r9, rcx
    shr r9, 11

  ; Move each 2048-byte block using AVX-512 registers, using R9 as
  ; a counter, and the `vmovdqu` and `vmovntdq` instructions

  .MoveBlockData:

    ; (The number of 2048-byte blocks left to copy is stored in R9, and
    ; decremented with each loop count; if it's zero, that means we're
    ; done, so exit the loop)

    cmp r9, 0
    je .MoveRemainder

    ; (Read thirty-two unaligned 64-byte blocks into the ZMM registers,
    ; using the `vmovdqu64` instruction - keep in mind that we're reading
    ; from [RSI]+n, which is the same as (*Source + n))

    vmovdqu64 zmm0, [rsi+0]
    vmovdqu64 zmm1, [rsi+64]
    vmovdqu64 zmm2, [rsi+128]
    vmovdqu64 zmm3, [rsi+192]
    vmovdqu64 zmm4, [rsi+256]
    vmovdqu64 zmm5, [rsi+320]
    vmovdqu64 zmm6, [rsi+384]
    vmovdqu64 zmm7, [rsi+448]
    vmovdqu64 zmm8, [rsi+512]
    vmovdqu64 zmm9, [rsi+576]
    vmovdqu64 zmm10, [rsi+640]
    vmovdqu64 zmm11, [rsi+704]
    vmovdqu64 zmm12, [rsi+768]
    vmovdqu64 zmm13, [rsi+832]
    vmovdqu64 zmm14, [rsi+896]
    vmovdqu64 zmm15, [rsi+960]
    vmovdqu64 zmm16, [rsi+1024]
    vmovdqu64 zmm17, [rsi+1088]
    vmovdqu64 zmm18, [rsi+1152]
    vmovdqu64 zmm19, [rsi+1216]
    vmovdqu64 zmm20, [rsi+1280]
    vmovdqu64 zmm21, [rsi+1344]
    vmovdqu64 zmm22, [rsi+1408]
    vmovdqu64 zmm23, [rsi+1472]
    vmovdqu64 zmm24, [rsi+1536]
    vmovdqu64 zmm25, [rsi+1600]
    vmovdqu64 zmm26, [rsi+1664]
    vmovdqu64 zmm27, [rsi+1728]
    vmovdqu64 zmm28, [rsi+1792]
    vmovdqu64 zmm29, [rsi+1856]
    vmovdqu64 zmm30, [rsi+1920]
    vmovdqu64 zmm31, [rsi+1984]

    ; (Write those values back to memory, using the `vmovntdq` instruction;
    ; this time, we're writing to [RDI+n], or (*Destination + n))

    vmovntdq [rdi+0], zmm0
    vmovntdq [rdi+64], zmm1
    vmovntdq [rdi+128], zmm2
    vmovntdq [rdi+192], zmm3
    vmovntdq [rdi+256], zmm4
    vmovntdq [rdi+320], zmm5
    vmovntdq [rdi+384], zmm6
    vmovntdq [rdi+448], zmm7
    vmovntdq [rdi+512], zmm8
    vmovntdq [rdi+576], zmm9
    vmovntdq [rdi+640], zmm10
    vmovntdq [rdi+704], zmm11
    vmovntdq [rdi+768], zmm12
    vmovntdq [rdi+832], zmm13
    vmovntdq [rdi+896], zmm14
    vmovntdq [rdi+960], zmm15
    vmovntdq [rdi+1024], zmm16
    vmovntdq [rdi+1088], zmm17
    vmovntdq [rdi+1152], zmm18
    vmovntdq [rdi+1216], zmm19
    vmovntdq [rdi+1280], zmm20
    vmovntdq [rdi+1344], zmm21
    vmovntdq [rdi+1408], zmm22
    vmovntdq [rdi+1472], zmm23
    vmovntdq [rdi+1536], zmm24
    vmovntdq [rdi+1600], zmm25
    vmovntdq [rdi+1664], zmm26
    vmovntdq [rdi+1728], zmm27
    vmovntdq [rdi+1792], zmm28
    vmovntdq [rdi+1856], zmm29
    vmovntdq [rdi+1920], zmm30
    vmovntdq [rdi+1984], zmm31

    ; (Unlike `rep movs*`, these instructions don't automatically increment
    ; or decrement registers for us, so we have to do that ourselves)

    add rsi, 2048
    add rdi, 2048

    sub rcx, 2048

    ; (Repeat the loop, decrementing our counter (R9))

    dec r9
    jmp .MoveBlockData

  ; Move the remainder of the data, using `rep movsb` again.

  .MoveRemainder:

    cld
    rep movsb

  ; Restore the previous state of the AVX registers, and return.

  .Cleanup:

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xrstor [_SimdRegisterArea]

    pop rdx
    pop rax

    ret
