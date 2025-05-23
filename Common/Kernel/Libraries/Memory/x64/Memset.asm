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
  je .MoveBlocks

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

  ; (Broadcast 32-bits to 64-bits, then move onto .MoveBlocks)

  .BroadcastDword:

    mov r10, rsi

    shl r10, 32
    or rsi, r10

  ; Once we're done with that, we can prepare to start storing blocks,
  ; using the `rep stosq` instruction.

  .MoveBlocks:

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

  .MoveRemainderLoop:

    ; (While R9 (the remainder) isn't zero, run this loop)

    cmp r9, 0
    je .Return

    ; (Store the 'current' byte (AL) in [RSI] using `stosb`)

    mov rcx, 1

    cld
    stosb

    ; (Move to the next byte, and decrement R9 (the number of bytes left
    ; to copy, before continuing the loop)

    shr rsi, 8
    dec r9

    jmp .MoveRemainderLoop

  ; (Now that we're done, we can return)

  .Return:
    ret



; This function shouldn't change any preserved registers.
; (void* Buffer (RDI), uint? Value (RSI), uint8 Width (RDX), uint64 Size (RCX))

_Memset_Sse2:

  ; (TODO - Placeholder since I haven't implemented this yet)
  jmp _Memset_Base
