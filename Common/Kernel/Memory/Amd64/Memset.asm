; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

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

    sfence
    ret



; This function shouldn't change any preserved registers, other than XMM
; ones, but those are covered by the System V ABI.

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

    ; (Calculate the amount of 64-byte blocks we'll need to fill out
    ; in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 6

    and rcx, (64 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Move RAX to the lower half of XMM0, and broadcast it to its
    ; higher half as well (using `punpcklqdq`))

    movq xmm0, rax
    punpcklqdq xmm0, xmm0

    ; (Use the `movdqa` instruction to copy XMM0's value to the
    ; first other four XMM registers.)

    movdqa xmm1, xmm0
    movdqa xmm2, xmm0
    movdqa xmm3, xmm0

  ; Fill each 64-byte block with SSE registers, using R9 as a
  ; counter (R9 == (Size / 64)).

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `movdqa` instruction to fill the next 64 bytes,
    ; while keeping XMM0 ~ XMM3 in cache (this is a temporal move))

    movdqa [rdi+0], xmm0
    movdqa [rdi+16], xmm1
    movdqa [rdi+32], xmm2
    movdqa [rdi+48], xmm3

    ; (Move to the next 64-byte block, and repeat the loop)

    add rdi, 64
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Now that we're done, let's return.

  .Cleanup:

    sfence
    ret



; This function shouldn't change any preserved registers, other than YMM
; ones, but those are covered by the System V ABI.

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

    ; (Calculate the amount of 128-byte blocks we'll need to fill out
    ; in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 7

    and rcx, (128 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Broadcast RAX to the rest of YMM0 (using `vbroadcastsd`))

    push rax
    vbroadcastsd ymm0, [rsp]
    pop rax

    ; (Use the `vmovdqa` instruction to copy YMM0's value to the
    ; first other four YMM registers.)

    vmovdqa ymm1, ymm0
    vmovdqa ymm2, ymm0
    vmovdqa ymm3, ymm0

  ; Fill each 128-byte block using AVX registers, using R9 as a
  ; counter (R9 == (Size / 128)).

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `vmovdqa` instruction to fill the next 128 bytes,
    ; while keeping YMM0 ~ YMM3 in cache (this is a temporal move))

    vmovdqa [rdi+0], ymm0
    vmovdqa [rdi+32], ymm1
    vmovdqa [rdi+64], ymm2
    vmovdqa [rdi+96], ymm3

    ; (Move to the next 128-byte block, and repeat the loop)

    add rdi, 128
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Now that we're done, we can safely return.

  .Cleanup:

    sfence
    ret



; This function shouldn't change any preserved registers, other than ZMM
; ones, but those are covered by the System V ABI.

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

    ; (Calculate the amount of 256-byte blocks we'll need to fill
    ; out in R9, and leave the remainder in RCX.)

    mov r9, rcx
    shr r9, 8

    and rcx, (256 - 1)

    ; (Broadcast AL to the rest of RAX, using R10 as a scratch register;
    ; this essentially means "copy the value of AL to all other bytes")

    mov r10, rax
    mov rax, 0101010101010101h
    imul rax, r10

    ; (Broadcast RAX to the rest of ZMM0 (using `vpbroadcastq`)

    vpbroadcastq zmm0, rax

    ; (Use the `vmovdqa64` instruction to copy ZMM0's value to the
    ; first other four ZMM registers.)

    vmovdqa64 zmm1, zmm0
    vmovdqa64 zmm2, zmm0
    vmovdqa64 zmm3, zmm0

  ; Fill each 256-byte block using AVX-512 registers, using R9 as a
  ; counter (R9 == (Size / 256)).

  .FillBlockData:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .FillRemainder

    ; (Otherwise, use the `vmovdqa64` instruction to fill the next 256
    ; bytes, while keeping ZMM0 ~ ZMM3 in cache (this is a temporal move))

    vmovdqa64 [rdi+0], zmm0
    vmovdqa64 [rdi+64], zmm1
    vmovdqa64 [rdi+128], zmm2
    vmovdqa64 [rdi+192], zmm3

    ; (Move to the next 256-byte block, and repeat the loop)

    add rdi, 256
    dec r9

    jmp .FillBlockData

  ; Fill out the remainder of the data, using `rep stosb` again.

  .FillRemainder:

    cld
    rep stosb

  ; Now that we're done, let's return.

  .Cleanup:

    sfence
    ret
