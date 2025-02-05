; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 09E00h] ; We are at 9E00h.
[BITS 32] ; This is 32-bit code.. for now.

; void prepareRealMode32()
;
; Inputs: (None)
; Outputs: (None)
;
; This is the beginning of our 'program' that's called by realMode() - it's right at 9E00h, and
; it's the code that's called by the realMode() function in RmWrapper.c.
;
; Right now, we're in 32-bit protected mode, like the rest of our 2nd stage bootloader. The
; purpose of this function is to prepare to downgrade ourselves to 16-bit protected mode, and so,
; we do the following:
;
; - Disable interrupts (to keep them from crashing the system)
;
; - Push all registers (and the flags) to the stack, to keep them from becoming affected by the
; rest of our code
;
; - Save our current stack pointer in the saveStack label, so we can preserve our stack
;
; - Load a 16-bit GDT (General Descriptor Table) suitable for real mode
;
; - Execute a far jump (setting CS to our code selector, 08h) to the next stage
;

prepareRealMode32:

  ; Disable interrupts (except NMIs); as we're switching between different modes, keeping them
  ; on *will* interfere with the process and potentially crash the system.

  cli

  ; Push all registers (and the flags) to the stack

  pushfd
  pushad

  ; Save our current ESP (stack pointer) in the saveStack label, to be retrieved later.

  mov [saveStack], esp

  ; Load our 16-bit GDT

  lgdt [realModeGdtDescriptor]

  ; Far jump to 16-bit protected mode (setting CS to our code selector, 08h)

  jmp 08h:prepareRealMode16


; void prepareRealMode16()
;
; Inputs: (None)
; Outputs: (None)
;
; We're now in 16-bit mode, but we're not in real mode yet - there are a couple things we need to
; do before jumping to our real mode payload.
;
; Unlike switching from real to protected mode, switching from protected to real mode involves an
; intermediate step where we're still executing 16-bit instructions, but in protected mode.
;
; This is because there are a few steps that need to be taken in 16-bit mode before switching to
; real mode. In this function, we:
;
; - Initialize the segment registers with our code (08h) and data (10h) selectors respectively
;
; - Load the 16-bit real mode IDT/IVT
;
; - Disable protected mode by clearing the first bit of CR0
;
; - Execute another far jump, this time to our 16-bit real mode code.
;
; Also, we tell our assembler that we're executing 16-bit code with [BITS 16], since we're now
; in 16-bit protected mode. This will also extend to our 16-bit real mode code.
;

[BITS 16]

prepareRealMode16:

  ; Load our (non-code) segment registers with our new GDT's data selector (10h).

  mov ax, 10h
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Load real mode IDT

  lidt [realModeIdtDescriptor]

  ; Disable protected mode by clearing the first bit of CR0.

  mov eax, cr0
  and eax, 0FFFFFFFEh
  mov cr0, eax

  ; Far jump to our 16-bit real mode code.
  ; As segment registers don't matter the way they do in protected mode, we set CS to 00h

  jmp 00h:initRealMode


; void initRealMode()
;
; Inputs: (None)
; Outputs: (None)
;
; We've finally made to a 16-bit real mode environment. Hooray! (This is legitimately hard lol)
;
; There are a few things we still need to do though; namely, we still need to reset the rest of
; our segment registers to 0 (they don't work the same way in real mode as they do in protected
; mode), and we also need to set up the stack.
;
; Because of how real mode addressing works, it's genuinely just much more convenient to set the
; stack to a location below FFFFh; we have our protected mode stack pointer (ESP) already saved
; in [saveStack], so we shouldn't need to worry about that.
;

initRealMode:

  ; Reset our (non-code) segment registers to 00h - don't worry, this works differently in real
  ; mode, we're not actually setting them to our GDT's null selector lol.

  mov ax, 00h
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  ; Set our stack pointer to 7C00h. Our protected mode stack pointer has been saved, so no
  ; worries.

  mov sp, 7C00h

  ; Jump to our real mode payload.

  jmp realModePayload


; void realModePayload()
;
; Inputs: (None)
; Outputs: (None)
;
; This is our real mode payload. Right now, we're in 16-bit real mode, and we have all of the
; regular BIOS interrupts available to us. This is a pretty similar environment to what we have
; with our bootsector.
;
; We essentially do three things in this stage:
;
; - First, we import data from the realModeTable struct at (start + E00h) (the value of the
; data at realModeTable->Eax is saved in eax, etc.)
;
; - Second, we disable the carry flag, enable interrupts, and execute the interrupt given in
; realModeTable->Int (which is at bp+26)
;
; - Third, we export data back to the realModeTable struct at (start + E00h) (including eflags),
; and then jump to the next stage (prepareProtectedMode16), which restores protected mode.
;

realModePayload:

  ; Import data from the realModeTable struct at (start + E00h).

  ; It's defined using __attribute__((packed)), so there's no padding between different values,
  ; and we know exactly where each field/attribute is located. Not only that, but its structure
  ; is also defined at the end of this file.

  mov eax, [realModeTable.Eax]
  mov ebx, [realModeTable.Ebx]
  mov ecx, [realModeTable.Ecx]
  mov edx, [realModeTable.Edx]

  mov si, [realModeTable.Si]
  mov di, [realModeTable.Di]

  mov bp, [realModeTable.Bp]

  push ax
  mov al, [realModeTable.Int]
  mov [saveInt], al
  pop ax

  mov bp, [realModeTable.Bp]

  mov ds, [realModeTable.Ds]
  mov es, [realModeTable.Es]

  ; Disable the carry flag, and enable interrupts. We've loaded the real mode IVT (located
  ; between 0 and 400h), so we can safely do this.

  clc
  sti

  ; Actually execute the given interrupt (stored in saveInt).

  ; We have to do something that's a little hacky. The 'int' instruction only takes immediate
  ; operands (like int 10h, int 3Fh, etc.), *not* memory locations (like int [saveInt]), or
  ; registers (like int [al]).

  ; Because we don't know what interrupt number we're going to call here until after compilation,
  ; we instead just manually emit the opcode for int, and then tell the code at the beginning of
  ; this function (which reads realModeTable->Int) that the saveInt label is located right
  ; after the int opcode, so that it calls the interrupt number we want it to.

  ; As an example - imagine you want to execute int 13h. You'd set realModeTable->Int to 13h,
  ; and then the previous code would write 13h to the saveInt label, which is right after CDh (the
  ; opcode for int), therefore resulting in CD 13h (int 13h).

  db 0CDh
  saveInt: db 0h

  ; Export data back to the realModeTable struct at (start + E00h), including eflags.

  ; It's defined using __attribute__((packed)), so there's no padding between different values,
  ; and we know exactly where each field/attribute is located.

  pusha

  mov ax, ds
  mov bx, es

  mov cx, 0
  mov ds, ax
  mov es, ax

  mov [realModeTable.Ds], ax
  mov [realModeTable.Es], bx

  popa

  mov [realModeTable.Eax], eax
  mov [realModeTable.Ebx], ebx
  mov [realModeTable.Ecx], ecx
  mov [realModeTable.Edx], edx

  mov [realModeTable.Si], si
  mov [realModeTable.Di], di

  mov [realModeTable.Bp], bp

  ; Use pushfd to also write the value of eflags to the struct. This is important, since it lets
  ; us check whether certain flags (like the carry flag / CF) are set later on.

  pushfd
  pop eax
  mov [realModeTable.Eflags], eax

  ; Jump to the stage that prepares us to go back to protected mode.

  jmp prepareProtectedMode16


; void prepareProtectedMode16()
;
; Inputs: (None)
; Outputs: (None)
;
; This section of the code prepares us to jump back into protected mode. We need to do this in
; two stages:
;
; - One in 16-bit real mode (which prepares the system for, and jumps to protected mode);
;
; - And another in 32-bit protected mode (which just sets up the 32-bit protected mode
; environment, letting us go back to our 2nd stage bootloader).
;
; This function represents the first stage of the process. We essentially just disable interrupts,
; load our 32-bit protected mode GDT (which is a copy of the one in Bootsector.asm), and then
; set the first bit of the CR0 register to actually enable protected mode.
;

prepareProtectedMode16:

  ; Disable interrupts; these can interfere with the process and cause the system to crash.

  cli

  ; Load our 32-bit protected mode GDT.

  ; We also reset the value of ds and es to prevent the system from crashing (since we're
  ; still in 16-bit mode, we're still using data segments to load things).

  mov ax, 0
  mov ds, ax
  mov es, ax

  lgdt [protectedModeGdtDescriptor]

  ; Enable protected mode by setting the first bit of the CR0 register.

  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; Do a far jump to the next stage (which is already in 32-bit protected mode), setting CS to
  ; our GDT's code selector (08h).

  jmp 08h:prepareProtectedMode32


; void prepareProtectedMode32()
;
; Inputs: (None)
; Outputs: (None)
;
; We're finally back in 32-bit protected mode. There are a few things we need to do before we can
; return to our 2nd stage bootloader though:
;
; - First, we need to set up the rest of the segment registers (DS, ES, FS, GS and SS) with our
; GDT's data selector (10h).
;
; - Second, we need to restore our stack pointer. We initially saved it at [saveStack] back in the
; prepareRealMode32 function, and we'll need to restore it so we can carry on normally.
;
; - Third, we need to pop the registers (and the flags) we initially pushed onto the stack in the
; aforementioned function, as to restore the system state.
;
; - Finally, after all of this, we can just execute the ret instruction to return back to our
; 2nd stage bootloader. Right now, the stack is in the same state as it was when we called the
; realMode() function in RmWrapper.c, so it's safe to do this.
;
; Also, we tell our assembler that we're executing 32-bit code again with [BITS 32], since we're
; now back in 32-bit protected mode.
;

[BITS 32]

prepareProtectedMode32:

  ; We're now in protected mode
  ; Restore segment vars

  mov eax, 10h
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax

  ; Restore our protected mode stack pointer, which was saved in [saveStack]

  mov esp, [saveStack]

  ; Restore all registers (and the flags), to restore the state we initially had when we called
  ; realMode() (and subsequently, the prepareRealMode32 function/label)

  popad
  popfd

  ; Just in case, turn off interrupts again, since we don't have an IDT loaded.

  cli

  ; Return with the ret instruction. Because of the C calling convention, this shouldn't be
  ; an issue.

  ret


; -----------------------------------

; This is where we'll reserve our stack pointer - although we save the state of every other
; register with the C calling conventions, we'll still need to know the stack pointer so we can
; actually pop those registers from the stack.

saveStack: dd 0

; -----------------------------------


; 16-bit real and protected mode GDT descriptor.

realModeGdtDescriptor:

  dw 24 - 1 ; We have three segments, each of which is 8 bytes long, so our GDT is 24 bytes.
  dd realModeGdt ; The location of our GDT is at the realModeGdt label.

; 16-bit real and protected mode GDT.

realModeGdt:

  ; Segment 0 (Null segment)

  dw 0000h ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 00000000b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 1 (Code segment, spans all 1MiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10011010b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

  ; Segment 2 (Data segment, spans all 1MiB of memory)

  dw 0FFFFh ; Limit (bits 0-15)
  dw 0000h ; Base (bits 0-15)
  db 00h ; Base (bits 16-23)
  db 10010010b ; Access byte (bits 0-7)
  db 00000000b ; Limit (bits 16-19) and flags (bits 0-7)
  db 00h ; Base (bits 24-31)

; 16-bit real mode IDT descriptor.

realModeIdtDescriptor:

  dw (400h - 1) ; The real-mode IDT is pretty much always 400h bytes long.
  dd 0 ; It's also usually at the very start of memory (0h).

; -----------------------------------

; 32-bit protected mode GDT descriptor.

protectedModeGdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd protectedModeGdt ; The location of our GDT is the protectedModeGdt label.

; 32-bit protected mode GDT.

protectedModeGdt:

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

; -----------------------------------

; This tells our assembler that we want the rest of our program (essentially any areas that haven't
; been used) to be filled with zeroes, up to the E00h (3584th) byte.

times 0E00h-($-$$) db 0

; The position of this table is guaranteed to always be E00h after the start of this file. It
; contains the information that we want to exchange with our protected-mode code; for example,
; the value we want to set the registers to, the interrupt being executed, etc.

realModeTable:

  realModeTable.Eax: dd 0
  realModeTable.Ebx: dd 0
  realModeTable.Ecx: dd 0
  realModeTable.Edx: dd 0

  realModeTable.Ds: dw 0
  realModeTable.Es: dw 0

  realModeTable.Si: dw 0
  realModeTable.Di: dw 0

  realModeTable.Bp: dw 0

  realModeTable.Eflags: dd 0

  realModeTable.Int: db 0
