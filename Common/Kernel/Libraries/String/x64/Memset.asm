; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

EXTERN SimdRegisterArea

GLOBAL Memset_RepStosb
GLOBAL Memset_Sse2
GLOBAL Memset_Avx
GLOBAL Memset_Avx512f


; This function shouldn't change any preserved registers.
; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

Memset_RepStosb:

  ; `rep stosb` is an instruction that fills RCX bytes at [RDI] with the
  ; value specified in AL, so let's map our registers accordingly:

  mov rax, rsi
  mov rcx, rdx

  ; If our size is less than 16 bytes, then not only is there little
  ; benefit to aligning to 16-byte boundaries, but we can also crash
  ; the system if Size > (Buffer % 16).

  cmp rdx, 16
  jl .FillAlignedData

  ; At this point, we're almost ready to fill bytes; we just need to
  ; align the buffer address (in RDI) to 16 bytes, if applicable.

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (16 - 1)

  mov r8, 16
  sub r8, r9

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
; ones, which requires `fxsave` support. (Size >= 256)

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

Memset_Sse2:

  ; First, let's check to see if the buffer address is 16-byte-aligned;
  ; and if not, use `rep stosb` to fill it out.)

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (16 - 1)

  mov r8, 16
  sub r8, r9

  ; (Store `Character` in RAX, and `Size` in RCX)

  mov rax, rsi
  mov rcx, rdx

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

    fxsave [SimdRegisterArea]

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
    ; other XMM register.)

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

    fxrstor [SimdRegisterArea]
    ret



; This function shouldn't change any preserved registers, other than YMM
; ones, which requires `xsave` support. (Size >= 512)

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

Memset_Avx:

  ; First, let's check to see if the buffer address is 32-byte-aligned;
  ; and if not, use `rep stosb` to fill it out.)

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (32 - 1)

  mov r8, 32
  sub r8, r9

  ; (Store `Character` in RAX, and `Size` in RCX)

  mov rax, rsi
  mov rcx, rdx

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 0
  je .FillAlignedData

  ; If the buffer address isn't 32-byte-aligned, copy the remainder
  ; (also using `rep stosb`, but *only* for the remainder)

  .FillUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep stosb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for AVX.

  .FillAlignedData:

    ; (Save the current state of the AVX registers, using `xsave`)

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xsave [SimdRegisterArea]

    pop rdx
    pop rax

    ; (Calculate the amount of 512-byte blocks we'll need to fill out
    ; in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 9

    and rcx, (512 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Broadcast RAX to the rest of YMM0 (using `vbroadcastsd`))

    push rax
    vbroadcastsd ymm0, [rsp]
    pop rax

    ; (Use the `vmovdqa` instruction to copy YMM0's value to every
    ; other YMM register.)

    vmovdqa ymm1, ymm0
    vmovdqa ymm2, ymm0
    vmovdqa ymm3, ymm0
    vmovdqa ymm4, ymm0
    vmovdqa ymm5, ymm0
    vmovdqa ymm6, ymm0
    vmovdqa ymm7, ymm0
    vmovdqa ymm8, ymm0
    vmovdqa ymm9, ymm0
    vmovdqa ymm10, ymm0
    vmovdqa ymm11, ymm0
    vmovdqa ymm12, ymm0
    vmovdqa ymm13, ymm0
    vmovdqa ymm14, ymm0
    vmovdqa ymm15, ymm0

  ; Fill each 512-byte block using AVX registers, using R9 as a
  ; counter (R9 == (Size / 512)). (Size >= 2048)

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `vmovdqa` instruction to fill the next 512 bytes,
    ; while keeping YMM0 ~ YMM15 in cache (this is a temporal move))

    vmovdqa [rdi+0], ymm0
    vmovdqa [rdi+32], ymm1
    vmovdqa [rdi+64], ymm2
    vmovdqa [rdi+96], ymm3
    vmovdqa [rdi+128], ymm4
    vmovdqa [rdi+160], ymm5
    vmovdqa [rdi+192], ymm6
    vmovdqa [rdi+224], ymm7
    vmovdqa [rdi+256], ymm8
    vmovdqa [rdi+288], ymm9
    vmovdqa [rdi+320], ymm10
    vmovdqa [rdi+352], ymm11
    vmovdqa [rdi+384], ymm12
    vmovdqa [rdi+416], ymm13
    vmovdqa [rdi+448], ymm14
    vmovdqa [rdi+480], ymm15

    ; (Move to the next 512-byte block, and repeat the loop)

    add rdi, 512
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Restore the previous state of the AVX registers, and return.

  .Cleanup:

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xrstor [SimdRegisterArea]

    pop rdx
    pop rax

    ret



; This function shouldn't change any preserved registers, other than ZMM
; ones, which requires `xsave` support.

; (void* Buffer (RDI), uint8 Character (RSI), uint64 Size (RDX))

Memset_Avx512f:

  ; First, let's check to see if the buffer address is 64-byte-aligned;
  ; and if not, use `rep stosb` to fill it out.)

  ; (We store the remainder in R8, and use R9 as a scratch register)

  mov r9, rdi
  and r9, (64 - 1)

  mov r8, 64
  sub r8, r9

  ; (Store `Character` in RAX, and `Size` in RCX)

  mov rax, rsi
  mov rcx, rdx

  ; (If it's already aligned, then we don't need to align it)

  cmp r8, 0
  je .FillAlignedData

  ; If the buffer address isn't 64-byte-aligned, copy the remainder
  ; (also using `rep stosb`, but *only* for the remainder)

  .FillUnalignedData:

    push rcx
    mov rcx, r8

    cld
    rep stosb

    pop rcx
    sub rcx, r8

  ; If it is, then prepare the environment for AVX-512.

  .FillAlignedData:

    ; (Save the current state of the AVX registers, using `xsave`)

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xsave [SimdRegisterArea]

    pop rdx
    pop rax

    ; (Calculate the amount of 2048-byte blocks we'll need to fill
    ; out in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 11

    and rcx, (2048 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Broadcast RAX to the rest of ZMM0 (using `vpbroadcastq`)

    vpbroadcastq zmm0, rax

    ; (Use the `vmovdqa64`` instruction to copy ZMM0's value to every
    ; other ZMM register.)

    vmovdqa64 zmm1, zmm0
    vmovdqa64 zmm2, zmm0
    vmovdqa64 zmm3, zmm0
    vmovdqa64 zmm4, zmm0
    vmovdqa64 zmm5, zmm0
    vmovdqa64 zmm6, zmm0
    vmovdqa64 zmm7, zmm0
    vmovdqa64 zmm8, zmm0
    vmovdqa64 zmm9, zmm0
    vmovdqa64 zmm10, zmm0
    vmovdqa64 zmm11, zmm0
    vmovdqa64 zmm12, zmm0
    vmovdqa64 zmm13, zmm0
    vmovdqa64 zmm14, zmm0
    vmovdqa64 zmm15, zmm0
    vmovdqa64 zmm16, zmm0
    vmovdqa64 zmm17, zmm0
    vmovdqa64 zmm18, zmm0
    vmovdqa64 zmm19, zmm0
    vmovdqa64 zmm20, zmm0
    vmovdqa64 zmm21, zmm0
    vmovdqa64 zmm22, zmm0
    vmovdqa64 zmm23, zmm0
    vmovdqa64 zmm24, zmm0
    vmovdqa64 zmm25, zmm0
    vmovdqa64 zmm26, zmm0
    vmovdqa64 zmm27, zmm0
    vmovdqa64 zmm28, zmm0
    vmovdqa64 zmm29, zmm0
    vmovdqa64 zmm30, zmm0
    vmovdqa64 zmm31, zmm0

  ; Fill each 2048-byte block using AVX registers, using R9 as a
  ; counter (R9 == (Size / 2048)).

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `vmovdqa64` instruction to fill the next 2048
    ; bytes, while keeping ZMM0 ~ ZMM32 in cache (this is a temporal move))

    vmovdqa64 [rdi+0], zmm0
    vmovdqa64 [rdi+64], zmm1
    vmovdqa64 [rdi+128], zmm2
    vmovdqa64 [rdi+192], zmm3
    vmovdqa64 [rdi+256], zmm4
    vmovdqa64 [rdi+320], zmm5
    vmovdqa64 [rdi+384], zmm6
    vmovdqa64 [rdi+448], zmm7
    vmovdqa64 [rdi+512], zmm8
    vmovdqa64 [rdi+576], zmm9
    vmovdqa64 [rdi+640], zmm10
    vmovdqa64 [rdi+704], zmm11
    vmovdqa64 [rdi+768], zmm12
    vmovdqa64 [rdi+832], zmm13
    vmovdqa64 [rdi+896], zmm14
    vmovdqa64 [rdi+960], zmm15
    vmovdqa64 [rdi+1024], zmm16
    vmovdqa64 [rdi+1088], zmm17
    vmovdqa64 [rdi+1152], zmm18
    vmovdqa64 [rdi+1216], zmm19
    vmovdqa64 [rdi+1280], zmm20
    vmovdqa64 [rdi+1344], zmm21
    vmovdqa64 [rdi+1408], zmm22
    vmovdqa64 [rdi+1472], zmm23
    vmovdqa64 [rdi+1536], zmm24
    vmovdqa64 [rdi+1600], zmm25
    vmovdqa64 [rdi+1664], zmm26
    vmovdqa64 [rdi+1728], zmm27
    vmovdqa64 [rdi+1792], zmm28
    vmovdqa64 [rdi+1856], zmm29
    vmovdqa64 [rdi+1920], zmm30
    vmovdqa64 [rdi+1984], zmm31

    ; (Move to the next 2048-byte block, and repeat the loop)

    add rdi, 2048
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Restore the previous state of the AVX registers, and return.

  .Cleanup:

    push rax
    push rdx

    mov rax, 0FFFFFFFFFFFFFFFFh
    mov rdx, 0FFFFFFFFFFFFFFFFh

    xrstor [SimdRegisterArea]

    pop rdx
    pop rax

    ret
