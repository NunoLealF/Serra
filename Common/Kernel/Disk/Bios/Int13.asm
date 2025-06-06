; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 7FE00h] ; We are at 7FE00h.
[BITS 64] ; This is 64-bit code.

; It's assumed that this wrapper will behave as a regular 64-bit function
; (using the SystemV ABI) would, so let's obtain our arguments:

; (uint64 Lba [rsi], uint32 NumSectors [rsi], uint8 DriveNumber [rdx ~ dl])

; (We preserve RBX, RSP, RBP, R12, R13, R14, R15 and *MM)
; (We return the value in RAX (on success, the lower 16 bits are 0)

Function_Int13Wrapper:
PrepareProtectedMode64:

  ; First, before we do anything else, let's store the necessary values
  ; in `DiskAddressPacket`, since it's easier to do that now.

  mov qword [DiskAddressPacket.Offset], rdi
  mov word [DiskAddressPacket.NumSectors], si

  ; Afterwards, let's disable interrupts, so they don't interfere with
  ; the process - we shouldn't need to disable NMIs however.

  cli

  ; Next, let's save the current value of the stack pointer (in RSP),
  ; as well as the GDT and IDT (in their respective labels).

  ; (Usually you'd just define a generic long mode GDT and IDT for
  ; performance reasons, but this code is located under 1 MiB, so
  ; there's a real risk of it getting overwritten)

  push rbx
  mov [SaveStack], rsp

  sgdt [LongModeGdtDescriptor]
  sidt [LongModeIdtDescriptor]

  ; Additionally, let's switch the stack pointer to a different
  ; location (in this case, 7C00h), so it's accessible across
  ; all execution modes:

  mov rsp, 7C00h

  ; Now, we should be ready to switch to protected mode, so let's
  ; update our GDT with 32-bit segments, and do a far return
  ; to protected-mode (setting CS).

  ; (We can't directly set CS with a far jump in long mode, so we
  ; need to push our preferred values for CS:IP, and then pop
  ; it with `retfq` - in this case, 08h:PrepareRealMode32)

  lgdt [ProtectedModeGdtDescriptor]

  push qword 08h
  push qword PrepareRealMode32
  retfq ; (`retfq` = `lretq` = `o64 retf`)



[BITS 32] ; This is 32-bit code.

PrepareRealMode32:

  ; Now that we're here, we should be in IA-32e (or 'compatibility')
  ; mode, which is essentially just a subset of protected mode.

  ; Before we switch back to regular protected mode though, let's
  ; set up the segment registers with our data segment (10h):

  mov bx, 10h

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  ; Next, let's save the current value of CR3 (since that contains
  ; our page tables) before we disable paging:

  mov ebx, cr3
  mov [SavePageTables], ebx

  mov ebx, cr0
  and ebx, ~(1 << 31)
  mov cr0, ebx

  ; Finally, let's disable compatibility mode altogether by *clearing*
  ; bit 8 of the EFER model specific register (at C0000080h).

  mov ecx, 0xC0000080
  rdmsr

  and eax, ~(1 << 8)
  wrmsr

  ; Now that we've done that, we *should* be in regular protected
  ; mode, which means we can actually start preparing to switch
  ; to 16-bit real mode.

  ; The first thing we need to do then is to load our GDT with
  ; 16-bit segments, before making a far jump to 16-bit protected
  ; mode (we can't switch directly):

  lgdt [RealModeGdtDescriptor]
  jmp 08h:PrepareRealMode16



[BITS 16] ; This is 16-bit code.

PrepareRealMode16:

  ; Right now, we're still in protected-mode, but using 16-bit
  ; segments, so our first action is to load those:

  mov bx, 10h

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  ; Now that we have, we can load the real mode IDT (or IVT,
  ; Interrupt Vector Table), although we can't call
  ; BIOS interrupts just yet.

  lidt [RealModeIdtDescriptor]

  ; Finally, we can disable protected mode (by *clearing* bit 0
  ; of CR0), and make a far jump to our 16-bit real mode code.

  ; (Our code starts at 7FE00h, so we should use 7FE0h as our
  ; segment within 16-bit real mode)

  mov ebx, cr0
  and ebx, ~(1 << 0)
  mov cr0, ebx

  jmp 7FE0h:(RealMode - 7FE00h)



RealMode:

  ; After all of this, we should be in 16-bit real mode, which means
  ; we can *finally* use BIOS interrupt calls.

  ; In this case, we want to use the BIOS interrupt (int 13h, AH =
  ; 42h, DL = (drive), DS:SI = &DiskAddressPacket) in order to
  ; read from the disk.

  ; (Disable the carry flag, and enable interrupts again)

  clc
  sti

  ; (Set DS:SI to the address of DiskAddressPacket; we also set
  ; ES, just in case.)

  mov bx, 7FE0h
  mov ds, bx
  mov es, bx

  mov si, (DiskAddressPacket - 7FE00h)

  ; (Set the AH register to 42h, and call interrupt 13h)

  mov ah, 42h
  mov al, 00h

  int 13h

  ; (Depending on whether the carry flag is set, either clear or set
  ; AL respectively, before jumping to `PrepareProtectedMode16`)

  mov al, 00h
  jnc PrepareProtectedMode16

  mov al, 0FFh
  jmp PrepareProtectedMode16



PrepareProtectedMode16:

  ; Now that we've finished executing our BIOS call, we can start
  ; returning back to protected (and eventually long) mode.

  ; Before we do anything else though, let's disable interrupts, since
  ; the real mode IDT/IVT isn't usable in protected mode:

  cli

  ; Next, let's load our GDT with 32-bit segments, and then re-enable
  ; protected mode by setting bit 0 of the CR0 register.

  ; (In 16-bit real mode, `lgdt` uses the DS register - thankfully,
  ; we already set that to 7FE0h earlier on, but it does mean that
  ; we need to *only* provide the offset)

  mov ebx, ProtectedModeGdtDescriptor
  sub ebx, 7FE00h
  shr ebx, 4

  lgdt [ebx]

  ; Finally, let's enable protected mode (by *setting* bit 0 of the
  ; CR0 register), and do a far jump to the next stage.

  mov ebx, cr0
  or ebx, (1 << 0)
  mov cr0, ebx

  jmp 08h:PrepareLongMode32



[BITS 32] ; This is 32-bit code.

PrepareLongMode32:

  ; We should finally be in 32-bit protected mode now. Before we do
  ; anything else though, let's restore the segment registers:

  ; (In this case, 10h is ProtectedModeGdt's data segment offset)

  mov bx, 10h

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  ; Next, let's restore the address of our page tables back to the
  ; CR3 register, like this:

  mov ebx, [SavePageTables]
  mov cr3, ebx

  ; After that, we can *set* bit 8 (IA-32e / 'compatibility') mode
  ; of the EFER model specific register (at C0000080h):

  ; (We temporarily save EAX in the EDX register here, because
  ; `rdmsr` and `wrmsr` both use it)

  mov edx, eax

  mov ecx, 0xC0000080
  rdmsr

  or eax, (1 << 8)
  wrmsr

  mov eax, edx

  ; Finally, we can enable (long mode-style) paging by setting bit
  ; 31 of CR0 - this *immediately* already re-enables paging.

  mov ebx, cr0
  or ebx, (1 << 31)
  mov cr0, ebx

  ; At this point, we're finally in IA-32e (compatibility) mode; in
  ; order to switch to long mode, we just need to update our GDT
  ; with 64-bit segments, and do a far jump.

  lgdt [LongModeGdtDescriptor]

  jmp 08h:ReturnFromWrapper



[BITS 64] ; This is 64-bit code.

ReturnFromWrapper:

  ; We're *finally* back in 64-bit long mode, so we're actually
  ; pretty close to being able to return to the kernel.

  ; For now, we still need to set up the segment registers (with
  ; 10h, which *should* be the data segment)

  mov bx, 0x10

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  ; Next, let's restore the old stack pointer, and then reload
  ; our kernel IDT (which we saved earlier on).

  ; (We don't know if it's safe to enable interrupts yet, so it's
  ; the caller's responsibility to enable them later on.)

  mov rsp, [SaveStack]
  pop rbx

  lidt [LongModeIdtDescriptor]

  ; Finally, let's return - in theory, the stack pointer should
  ; be identical to when we called it, so this should be fine.

  ret



; ----------------------------------------------------------------------

; In order to read sectors from disk using the (int 13h, ah = 42h)
; BIOS call, we need to pass on a disk address packet:

DiskAddressPacket:
  .Size: db 16 ; (The size of this packet is 16 bytes)
  .Reserved: db 0h ; (Not used)
  .NumSectors: dw 0h ; (How many sectors should we load?)
  .Location: dd 70000h ; (Tell the firmware to load sectors to 70000h)
  .Offset: dq 0h ; (From which LBA offset should we start loading?)

; In order to avoid corrupting the stack, we'll need to save the stack
; pointer (the value of RSP) early on, so let's do that:

SaveStack:
  dq 0h

; Finally, we'll also need to save the page tables (the value of CR3)
; as we switch out of long mode, so let's reserve space for that:

SavePageTables:
  dq 0h


; ----------------------------------------------------------------------

; Throughout the wrapper, we'll need to switch between 64-bit (long),
; 32-bit (protected) and 16-bit (real) mode, which means we'll need to
; have three different GDTs (Global Descriptor Tables) for each.

; (Define descriptors for all three GDTs - keep in mind that we don't
; define a long mode GDT here, since we'll just use `sgdt`)

RealModeGdtDescriptor:
  .Size: dw (RealModeGdt.End - RealModeGdt - 1)
  .Address: dd RealModeGdt

ProtectedModeGdtDescriptor:
  .Size: dw (ProtectedModeGdt.End - ProtectedModeGdt - 1)
  .Address: dq ProtectedModeGdt ; (Must be 8 bytes, so we can load it from 64-bit mode.)

LongModeGdtDescriptor:
  .Size: dw 0h
  .Address: dq 0h

; (Our 16-bit real mode GDT, with null + code + data segments)

RealModeGdt:

  ; [Segment 0 (null segment, spans nothing)]

  .Null:

    dw 0000h ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 00000000b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 1 (code segment, spans the first 1 MiB of memory)]

  .Code:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10011010b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 2 (data segment, spans the first 1 MiB of memory)]

  .Data:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10010010b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  .End:

; (Our 32-bit protected mode GDT, with null + code + data segments)

ProtectedModeGdt:

  ; [Segment 0 (null segment, spans nothing)]

  .Null:

    dw 0000h ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 00000000b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 1 (code segment, spans the first 4 GiB of memory)]

  .Code:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10011010b ; Access byte (bits 0-7)
    db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 2 (data segment, spans the first 4 GiB of memory)]

  .Data:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10010010b ; Access byte (bits 0-7)
    db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  .End:

; Additionally, we also need to define an IDT descriptor for when we're
; in real mode (so we can access BIOS interrupts), like this:

RealModeIdtDescriptor:
  .Size: dw 3FFh
  .Location: dd 0h

; Finally, this is where we'll save the long mode IDT (with `sidt`):

LongModeIdtDescriptor:
  .Size: dw 0h
  .Location: dq 0h



; ----------------------------------------------------------------------

; (Tell our assembler that we want to pad the rest of our binary with zeroes
; up to the 512th byte (up to 80000h))

times 512 - ($-$$) db 0

; Since this is included with #embed (rather than linked), in order for
; Setup_Int13Wrapper() to correctly validate this wrapper, we also need to
; include a signature and some extra information at the end.

; Keep in mind that this won't be copied over to 7FE00h - only the first
; 512 bytes will - it'll just be used for verification, so the above
; code can't use these variables

.Int13Wrapper_Signature:
  dq 3331496172726553h

.Int13Wrapper_Data:
  dd 70000h

.Int13Wrapper_Location:
  dd 7FE00h
