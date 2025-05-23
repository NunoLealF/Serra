; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 64]

DEFAULT REL
SECTION .text

EXTERN _SimdRegisterArea

GLOBAL _Memset_Base
GLOBAL _Memset_Sse2


; This function shouldn't change any preserved registers.
; (void* Buffer (RDI), uint? Value (RSI), uint8 Width (RDX), uint64 Size (RCX))

_Memset_Base:

  ; Calculate the number of 8-byte blocks we need to set in R8, and store
  ; the remainder of that calculation (number of 1-byte blocks?) in R9.

  ; (In other words, r8 => (Size / 8), r9 => (Size % 8))

  mov r8, rcx
  shr r8, 3

  and r9, 7

  ; Depending on the value of Width ( = sizeof(Value)), broadcast the
  ; rest of Value into RSI (using R10 as a scratch register).

  ; (For reference, broadcasting just means filling out the unused 'parts'
  ; of a register with the used 'parts', like in A1h -> A1A1A1A1A1A1A1A1h)

  cmp rdx, 1
  je .BroadcastByte

  cmp rdx, 2
  je .BroadcastWord

  cmp rdx, 4
  je .BroadcastDword

  cmp rdx, 8
  je .BlockFill

  ; (Broadcast 8-bits to 16-bits, then move onto .BroadcastWord)

  .BroadcastByte:
    mov r10, rsi
    shl r10, 8
    or rsi, r10

  ; (Broadcast 16-bits to 32-bits, then move onto .BroadcastDword)

  .BroadcastWord:
    mov r10, rsi
    shl r10, 16
    or rsi, r10

  ; (Broadcast 32-bits to 64-bits, then move onto .BlockFill)

  .BroadcastDword:
    mov r10, rsi
    shl r10, 32
    or rsi, r10

  ; Once we're done with that, we can prepare to start storing blocks,
  ; using the `rep stosq` instruction.

  .BlockFill:

    ; (Store our value in RAX, our buffer in RDI, and the number of
    ; blocks to fill in RCX)

    mov rax, rsi
    mov rcx, r8

    ; (Use `rep stosq` to store RCX 8-byte blocks (of value RAX) at [RSI])

    cld
    rep stosq

  ; Since the Size variable isn't guaranteed to be a multiple of eight, we
  ; also have a remainder number of bytes that we need to store, so let's
  ; deal with those using `stosb`.

  .RemainderFill:

    ; (While R9 (the remainder) isn't zero, run this loop)

    cmp r9, 0
    je .Cleanup

    ; (Store the 'current' byte (AL) in [RSI] using `stosb`)

    mov rcx, 1

    cld
    stosb

    ; (Move to the next byte, and decrement R9 (the number of bytes left
    ; to copy, before continuing the loop)

    shr rsi, 8
    dec r9

    jmp .RemainderFill

  ; (Now that we're done, we can return)

  .Cleanup:
    ret



; This function shouldn't change any preserved registers, other than XMM
; ones, which requires `fxsave` support.

; (void* Buffer (RDI), uint? Value (RSI), uint8 Width (RDX), uint64 Size (RCX))

_Memset_Sse2:

  ; Before we do anything else, we need to make sure that the address we're
  ; using is 16-byte-aligned; we'll store the remainder in R8.

  mov r8, rdi
  and r8, 15

  ; (If it's already aligned, skip trying to align it)

  cmp r8, 0
  je .AfterAlignment

  ; (Use _Memset_Base to fill the non-aligned bytes (R10) - this discards
  ; RCX, RDI, RSI and R8, so we need to save those on the stack)

  .AlignBuffer:

    push rcx
    push rdi
    push rsi
    push r8

    mov rcx, r8
    call _Memset_Base

    pop r8
    pop rsi
    pop rdi
    pop rcx

    ; (Update Size (RCX) and Buffer (RDI) to take into account the
    ; remainder we just copied (R8)).

    add rdi, 16
    sub rdi, r8
    sub rcx, r8

  ; (Prepare the environment for SSE)

  .AfterAlignment:

    ; Calculate the number of 256-byte blocks we need to fill in R9, and store
    ; the remainder (the number of bytes *after*) in RCX.

    mov r9, rcx
    shr r9, 8

    and rcx, 255

    ; Save the current state of the SSE registers, using `fxsave`.

    fxsave [_SimdRegisterArea]

    ; Depending on the value of Width, broadcast Value to the lower half
    ; of the XMM0 register.

    cmp rdx, 1
    je .BroadcastByte

    movq xmm0, rsi

    cmp rdx, 2
    je .BroadcastWord

    cmp rdx, 4
    je .BroadcastDword

    cmp rdx, 8
    je .BroadcastQword

  ; (Broadcast 8-bits to 16-bits in RSI -> XMM0, and move onto .BroadcastWord)

  .BroadcastByte:
    mov r10, rsi
    shl r10, 8
    or rsi, r10

    movq xmm0, rsi
    jmp .BroadcastWord

  ; (Broadcast 16-bits to 64-bits in XMM0, and move onto .BroadcastQword)

  .BroadcastWord:
    pshuflw xmm0, xmm0, 0b00000000
    jmp .BroadcastQword

  ; (Broadcast 32-bits to 64-bits in XMM0, and move onto .BroadcastQword)

  .BroadcastDword:
    pshuflw xmm0, xmm0, 0b01000100
    jmp .BroadcastQword

  ; (Broadcast 64-bits to 128-bits in XMM0, and move onto .BroadcastXmm)

  .BroadcastQword:
    punpcklqdq xmm0, xmm0
    jmp .BroadcastXmm

  ; (Broadcast to the full set of XMM registers, and move onto .SseBlockFill)

  .BroadcastXmm:
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

  ; Earlier on, we calculated that we'd need to fill R9 256-byte blocks;
  ; now that we've broadcasted Value to every XMM register, we can start
  ; filling them.

  .SseBlockFill:

    ; (If we've filled all necessary blocks, exit the loop)

    cmp r9, 0
    je .RemainderFill

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

    jmp .SseBlockFill

  ; After that, use `rep stosq` to fill the remaining (RCX) bytes (in this
  ; case, we can assume that RDI and RCX are both 8-byte aligned).

  .RemainderFill:

    ; (Set up RAX and RCX so we can properly use `rep stosq`, and
    ; call it; this should fill the remainder of our buffer.)

    movq rax, xmm0
    shr rcx, 3

    cld
    rep stosq

  .Cleanup:

    ; (Restore the previous state of the SSE registers, using `fxrstor`,
    ; and return)

    fxrstor [_SimdRegisterArea]
    ret
