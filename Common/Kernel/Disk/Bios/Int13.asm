; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 1000h] ; We are at 1000h.
[BITS 64] ; This is 64-bit code (for now).

; It's assumed that this wrapper will behave as a regular 64-bit function
; (using the SystemV ABI) would, so let's obtain our arguments:

; (uint64 Lba [rdi], uint16 Sectors [si], uint8 DriveNumber [dl])

; (We preserve RBX, RSP, RBP, R12, R13, R14, R15 and *MM)
; (We return the value in RAX (on success, the lower 16 bits are 0)

PrepareProtectedMode64:

  ; First, before we do anything else, let's store the necessary values
  ; in `DiskAddressPacket`, since it's easier to do that now.

  mov [DiskAddressPacket.Offset], rdi
  mov [DiskAddressPacket.NumSectors], si

  ; Afterwards, let's disable interrupts, so they don't interfere with
  ; the process - we shouldn't need to disable NMIs however.

  cli

  ; Next, let's save the current value of the stack pointer (in RSP),
  ; as well as the GDT and IDT (in their respective labels).

  ; (Usually you'd just define a generic long mode GDT and IDT for
  ; performance reasons, but this code is located under 1 MiB, so
  ; there's a real risk of it getting overwritten)

  push rbx
  push rbp
  push r12
  push r13
  push r14
  push r15

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



[BITS 32] ; This is 32-bit code (for now).

PrepareRealMode32:

  ; Now that we're here, we should be in IA-32e (or 'compatibility')
  ; mode, which is essentially just a subset of protected mode.

  ; Before we switch back to regular protected mode though, let's load
  ; the segment registers with our 32-bit data segment, 10h:

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

  ; (Since `wrmsr`/`rdmsr` uses EDX, we need to push it to the stack)

  push edx

  mov ecx, 0xC0000080
  rdmsr

  and eax, ~(1 << 8)
  wrmsr

  pop edx

  ; Now that we've done that, we *should* be in regular protected mode,
  ; which means we can start the process of switching to real mode.

  ; The first thing we need to do then, is to reload the segment
  ; registers with our 16-bit code selector (18h) by making a
  ; far jump to `PrepareRealMode16`, like this:

  jmp 18h:PrepareRealMode16



[BITS 16] ; This is 16-bit code (for now).

PrepareRealMode16:

  ; Right now, our code segment is in 16-bit protected mode, but our other
  ; segments aren't, so we need to load our segment registers:

  mov bx, 20h

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  ; Now that we have, we can load the real mode IDT (or IVT, Interrupt
  ; Vector Table), although we can't call BIOS interrupts just yet.

  lidt [RealModeIvtDescriptor]

  ; Finally, we can disable protected mode (by *clearing* bit 0 of CR0),
  ; and make a far jump to our 16-bit real mode payload.

  ; (Unlike protected mode, real mode uses segments completely differently,
  ; so we actually use 0h as our base segment within real mode)

  mov ebx, cr0
  and ebx, ~(1 << 0)
  mov cr0, ebx

  jmp 0h:RealMode



RealMode:

  ; After all of this, we should finally be in 16-bit real mode, which
  ; means we're ready to use BIOS interrupt calls.

  ; Before we do that though, let's reset the segments (to 0h as well),
  ; before disabling the carry flag and enabling interrupts:

  mov bx, 0h

  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  mov ss, bx

  clc
  sti

  ; In this case, we want to use the BIOS interrupt (int 13h, AH =
  ; 42h, DL = (drive), DS:SI = &DiskAddressPacket) in order to
  ; read from the disk.

  ; (Set DS:SI to &DiskAddressPacket)

  mov si, DiskAddressPacket

  ; (Set the AH register to 42h, and call interrupt 13h)
  ; We don't need to set DL, since it's been preserved so far

  mov ah, 42h
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
  clc

  ; We don't need to reload our GDT again, since it already has both 16
  ; and 32-bit segments, so we can already enable protected mode:

  ; (We enable protected mode by *setting* bit 0 of the CR0 register)

  mov ebx, cr0
  or ebx, (1 << 0)
  mov cr0, ebx

  ; Finally, let's make a far jump to set the code segment to 08h
  ; (our GDT's 32-bit code segment), like this:

  jmp 08h:PrepareLongMode32



[BITS 32] ; This is 32-bit code (for now).

PrepareLongMode32:

  ; We should finally be in 32-bit protected mode now. Before we do
  ; anything else though, let's restore the segment registers:

  ; (In this case, our 32-bit data segment is at an offset of 10h)

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

  ; (Since `wrmsr`/`rdmsr` uses EAX, we need to push it to the stack)

  push eax

  mov ecx, 0xC0000080
  rdmsr

  or eax, (1 << 8)
  wrmsr

  pop eax

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

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbp
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
  .Reserved: db 0h ; (Unused, can safely ignore)
  .NumSectors: dw 0h ; (How many sectors should we load?)
  .Location.Offset: dw 0h ; (Tell the firmware to load sectors to 7000:[0000h] = 70000h)
  .Location.Segment: dw 7000h ; (Tell the firmware to load sectors to [7000]:0000h = 70000h)
  .Offset: dq 0h ; (From which LBA should we start loading?)


; In order to avoid corrupting the stack, we'll need to save the stack
; pointer (the value of RSP) early on, so let's do that:

SaveStack:
  dq 0h


; Finally, we'll also need to save the page tables (the value of CR3)
; as we switch out of long mode, so let's reserve space for that:

SavePageTables:
  dq 0h


; ----------------------------------------------------------------------

; Throughout the wrapper, we'll need to switch between 64-bit (long)
; and 16-/32-bit (protected) mode, which means we'll need to have
; two separate GDTs (Global Descriptor Tables) for each.

; (Define descriptors for both GDTs - keep in mind that we don't define
; a long mode GDT here, since we'll save the current one with `sgdt`)

ProtectedModeGdtDescriptor:
  .Size: dw (ProtectedModeGdt.End - ProtectedModeGdt - 1)
  .Address: dq ProtectedModeGdt ; (Must be 8 bytes, so we can load it from 64-bit mode.)

LongModeGdtDescriptor:
  .Size: dw 0h ; (To be filled out by `sgdt`)
  .Address: dq 0h ; (To be filled out by `sgdt`)


; We'll also need to switch between our kernel's IDT, and the real mode
; IVT (Interrupt Vector Table), so let's define descriptors for both:

RealModeIvtDescriptor:
  .Size: dw 3FFh ; (The real mode IVT is always 400h bytes long)
  .Location: dd 0h ; (And located at address 0h)

LongModeIdtDescriptor:
  .Size: dw 0h ; (To be filled out by `sidt`)
  .Location: dq 0h ; (To be filled out by `sidt`)


; Finally, let's declare our protected-mode GDT, with both 16- and 32-
; bit code and data segments (08h, 10h, 18h and 20h respectively):

ProtectedModeGdt:

  ; [Segment 0 (null segment, spans nothing)]

  .Null:

    dw 0000h ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 00000000b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 1 (32-bit code segment, spans the first 4 GiB of memory)]

  .Code32:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10011010b ; Access byte (bits 0-7)
    db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 2 (32-bit data segment, spans the first 4 GiB of memory)]

  .Data32:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10010010b ; Access byte (bits 0-7)
    db 11001111b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 3 (16-bit code segment, spans the first 64 KiB of memory)]

  .Code16:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10011010b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  ; [Segment 4 (16-bit data segment, spans the first 64 KiB of memory)]

  .Data16:

    dw 0FFFFh ; Limit (bits 0-15)
    dw 0000h ; Base (bits 0-15)
    db 00h ; Base (bits 16-23)
    db 10010010b ; Access byte (bits 0-7)
    db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
    db 00h ; Base (bits 24-31)

  .End:



; ----------------------------------------------------------------------

; (Tell our assembler that we want to pad the rest of our binary with zeroes
; up to the 512th byte (up to 80000h))

times 512 - ($-$$) db 0


; Since this is included with #embed (rather than linked), in order for
; Setup_Int13Wrapper() to correctly validate this wrapper, we also need to
; include a signature and some extra information at the end.

; Keep in mind that this won't be copied over to 1000h - only the first
; 512 bytes will - it'll just be used for verification, so the above
; code can't use these variables

.Int13Wrapper_Signature:
  dq 3331496172726553h

.Int13Wrapper_Data:
  dd 70000h

.Int13Wrapper_Location:
  dd 1000h
