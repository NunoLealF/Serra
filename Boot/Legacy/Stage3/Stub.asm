; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[BITS 32]

SECTION .text

EXTERN KernelEntrypoint
EXTERN KernelStack
GLOBAL TransitionStub

; It's assumed that this stub will behave as a regular 32-bit function
; would, so we need to initialize a new call frame, and get arguments:

; (commonInfoTable* InfoTable [ebp+8], void* Pml4 [ebp+12])
; (We preserve EBP, ESP, EBX, ECX, ESI and EDI.)
; (We return in EDX and EAX.)

TransitionStub:

  ; Deal with the C calling convention / initialize the call frame.

  push ebp
  mov ebp, esp

  ; Set up the necessary environment - disable interrupts, and push
  ; everything to the stack.

  cli
  pushad

  ; Move InfoTable to edi, and Pml4 to esi; we can't do this before
  ; we push to the stack, otherwise we don't preserve esi/edi.

  mov edi, [ebp + 8]
  mov esi, [ebp + 12]

  ; Store our current stack pointer, so we can restore it later.

  mov [SaveStack], esp

  ; Enable page address extensions (PAE), by setting bit 5 of CR4.

  mov eax, cr4
  or eax, (1 << 5)
  mov cr4, eax

  ; Store the address of our PML4 in the CR3 register (making sure to
  ; *clear* bits 0-11; preserving them can cause it to crash).

  and esi, 0xFFFFF000
  mov cr3, esi

  ; Get the current value of the EFER register, and set bit 8 (long mode)

  mov ecx, 0xC0000080
  rdmsr

  or eax, (1 << 8)
  wrmsr

  ; Enable (long mode) paging by setting bit 31 of CR0.

  mov eax, cr0
  or eax, (1 << 31)
  mov cr0, eax

  ; At this point, we're finally in compatibility (IA-32e) mode, but
  ; in order to switch to long mode, we need to:

  ; (1) Update the GDT with 64-bit segments;

  lgdt [LongModeGdtDescriptor]

  ; (2) Set the CS register by doing a far jump (in this case, 08h
  ; represents our code segment) - this "resets" our GDT.

  jmp 0x08:JumpToKernel


; ------------------------------------------------------------

[BITS 64]

JumpToKernel:

  ; Now that we're here, our CPU has successfully entered long
  ; mode - hooray! <3

  ; We still need to set up a couple things before we can jump
  ; to the actual entrypoint, though.

  ; (3) Set up the segment registers with 10h (our data segment)

  mov ax, 0x10

  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; (4) Set up the stack (we subtract 128 bytes just in case)

  mov rsp, [KernelStack]
  sub rsp, 128

  ; (5) Set up the call frame (the stack needs to be 16-byte
  ; aligned, so we subtract 8 bytes before pushing rbp)

  sub rsp, 8
  push rbp

  ; Now that we've done that, we can finally jump to the
  ; kernel entrypoint:

  mov rcx, [KernelEntrypoint]
  call rcx

ReturnFromKernel64:

  ; If we're here, then that means that the entrypoint returned,
  ; so we need to transfer back control to our bootloader.

  ; First though - the 32-bit System-V ABI requires that 64-bit
  ; values be returned in EDX:EAX, whereas the 64-bit ABI just
  ; returns them in RAX, so we need to do this:

  mov rdx, rax
  shr rdx, 32

  ; We also want to save them, since they'll be reset by some
  ; upcoming instructions:

  mov [SaveEax], eax
  mov [SaveEdx], edx

  ; Okay - now we can actually make the switch back to 32-bit
  ; mode. First though, we need to switch from long mode to
  ; compatibility mode (IA-32e), like this:

  ; (1) Update the GDT with 32-bit segments;

  lgdt [ProtectedModeGdtDescriptor]

  ; (2) Set the CS register by doing a far return, which pops
  ; CS and IP from the stack, "resetting" our GDT.

  ; Keep in mind that long mode stores CS as a 64-bit register,
  ; so we *must* push it as a qword, because retfq pops it as
  ; one.

  push qword 0x08
  push qword ReturnFromKernel32
  retfq ; (`retfq` = `lretq` = `o64 retf`)


; ------------------------------------------------------------

[BITS 32]

ReturnFromKernel32:

  ; Now that we're back in 32-bit mode, we just need to unwind
  ; the changes we made to get here.

  ; (1) Set up the segment registers with our data segment (10h),
  ; again.

  mov ax, 0x10

  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; (2) Disable long mode paging, by clearing bit 31 of CR0

  mov eax, cr0
  and eax, ~(1 << 31)
  mov cr0, eax

  ; (3) Disable compatibility mode, by clearing bit 8 of the
  ; EFER model specific register (MSR number C0000080h)

  mov ecx, 0xC0000080
  rdmsr

  and eax, ~(1 << 8)
  wrmsr

  ; We don't really need to deal with CR3 or disable PAE, since
  ; those steps don't affect anything in protected mode, so we
  ; can restore the previous program state and return.

  ; (4) Restore the old stack pointer from [SaveStack].

  mov esp, [SaveStack]

  ; (5) Restore the registers we pushed at the beginning of
  ; TransitionStub.

  popad
  sti

  ; (6) Restore the value of EDX and EAX from [SaveEdx] and
  ; [SaveEax], since that has our return value.

  mov edx, [SaveEdx]
  mov eax, [SaveEax]

  ; (7) Pop the base pointer from the stack, and return.

  mov esp, ebp
  pop ebp
  ret


; ------------------------------------------------------------

; A space for some of our original 32-bit registers, if we
; ever need to preserve/save/restore them.

SaveEax:
  dd 0

SaveEdx:
  dd 0

SaveStack:
  dd 0


; ------------------------------------------------------------

; Our 32-bit protected mode GDT.

ProtectedModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all 4GiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all 4GiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

; Our 32-bit protected mode GDT descriptor.

ProtectedModeGdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd ProtectedModeGdt ; The location of our GDT is the ProtectedModeGdt label.


; ------------------------------------------------------------

; Our 64-bit GDT.

LongModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all of memory *no matter what*)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 10101111b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all of memory *no matter what*)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 11001111b ; Limit (bits 16-19) and flags (bits 0-3)
  db 00h ; Base (bits 24-31)

; Our 64-bit GDT descriptor.

LongModeGdtDescriptor:

  .Size: dw (LongModeGdtDescriptor - LongModeGdt - 1) ; sizeof(LongModeGdt) - 1
  .Address: dq LongModeGdt ; The location of LongModeGdt
