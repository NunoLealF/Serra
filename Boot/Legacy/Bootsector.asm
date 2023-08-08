; Copyright (C) 2023 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 07C00h] ; We are at 7C00h.
[BITS 16] ; This is 16-bit code.


; The start of our bootsector. These two instructions correspond to three bytes, which jump
; to a set position (+120 bytes) after the BIOS Parameter Block - 'EB 76 90'.

jmp short 120
nop


; The first 120 bytes of our bootsector isn't dedicated to storing actual code, but rather the
; BIOS Parameter Block (also known as the BPB), which is a table that contains important
; information about the (FAT) filesystem on the device.

; In order to maintain compatibility with some systems (and also with EFI), we will be providing
; a limited FAT filesystem for our bootloader. This means that we will need to fill out the
; BPB. However, as of now, only the bootable flag (40h bytes in) is set.

BPB:
  times 63 db 0
  db 40
  times 53 db 0


; Now that we've passed over the BPB, we're back to actually executing code. Hooray!

; At the moment, we're in 16-bit real mode, which means that we're currently using the real mode
; addressing system. However, that means we don't know whether CS:IP is set to 0000:7C00h, or
; if it's something closer to, say, 07C0:0000h.

; In order to avoid issues with our code later on, we'll be doing a far jump to set CS to 0
; (therefore resetting it to the 0000:7C00h format).

SetCS:

  jmp 00h:Start


; After all this, we've finally gotten to the actual start of our bootsector!
; We'll start off by setting up the segment registers (we only set CS to 00h, not DS, ES or
; SS) and the stack, before moving on to load the second stage bootloader into memory.

Start:

  ; Before we do anything, it's important that we turn off interrupts - not all segment registers
  ; are properly set at the moment!

  cli

  ; Now we can move on to actually setting up the segment registers (DS, ES, SS), along with
  ; the stack.

  ; We set up the stack at 7C00h, as that location should be available on essentially *all*
  ; x86 systems, and because it'll leave the stack with around 30KiB of space, which should
  ; be more than enough.

  mov ax, 00h
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 7C00h

  ; We can finally reenable interrupts now.

  sti

  ; The next step of the process is to try to load the second stage bootloader, and we'll
  ; allow up to 8 tries for this.

  ; Although this process 'uses up' most registers, si should be free, and so we'll be
  ; using it to keep track of how many times we've tried.

  mov si, 0

  ; We can now finally jump to loadDisk!

  jmp loadDisk


; Now that we've set up a basic environment for our bootloader (segment registers and stack
; have been set up, interrupts are on), we can attempt to load the second stage bootloader.

; We'll be doing this using the BIOS interrupt call int 13h / ah 02h, which is available on
; most systems, and which allows us to load up to 3Fh (63) sectors at a time.

; Additionally, we'll also try to do this eight times before 'giving up'. This is because some
; older systems may actually require multiple tries in order to successfully load all of the
; sectors.

loadDisk:

  ; Use the BIOS interrupt call int 13h / ah 02h to load the second stage bootloader.

  ; Specifically, we want to load the next 3Fh (sixty three) sectors at 7E00h, from the
  ; first head / cylinder, and the current drive number (already stored in dl).

  mov ah, 02h
  mov al, 3Fh
  mov bx, 7E00h

  mov ch, 0
  mov cl, 2

  mov dh, 0

  int 13h

  ; At this moment, there are two possibilities - either the data was loaded successfully
  ; (and it's safe to proceed to the next stage), or it failed to load properly. This is
  ; indicated by the carry flag.

  ; If the data was loaded successfully, we'll proceed to the protectedMode label, which
  ; should initialize 32-bit protected mode and then jump to 7E00h.

  jnc protectedMode ; If it's loaded, initialize protected mode and then jump to 7E00h

  ; If it wasn't, we'll increment si (our 'counter' from before), and check to see if
  ; it's over eight tries yet.

  ; If it isn't, we'll try to load the next stage again, but if it *is*, we'll jump to the
  ; x86 reset vector, which is usually located at 0FFFFh:0.

  inc si
  cmp si, 8

  jg loadDisk
  jmp 0FFFFh:0


; By now, we've loaded the next stage of our bootloader into memory. However, there's still
; one more thing we need to sort out - we're currently in 16-bit real mode, while our second
; stage bootloader is made for 32-bit protected mode!

; Therefore, we'll be setting up a (bare-bones) 32-bit protected mode environment, before
; actually jumping to our second stage at 7E00h. Specifically, we need to do two things:
; - Load a suitable protected mode GDT (Global Descriptor Table)
; - Set the protected mode enable ('PE') bit of the first control register, CR0.

; After all of that, we just need to do a far jump to 7E00h, setting CS to our code selector
; (08h), finally moving to our second-stage bootloader.

; Update/edit: (We'll also try to enable A20 here, before doing anything; not guaranteed to
; work, but hey, /shrug)

protectedMode:

  ; Try to enable A20 via the int 15h, ax 2401h BIOS interrupt. Not guaranteed to work, but
  ; it might, so..

  mov ax, 2401h
  int 15h

  ; As we'll be messing with the GDT and enabling protected mode, we'll definitely need to
  ; disable interrupts to prevent things from going south.

  cli

  ; Load [gdtDescriptor], which should contain a suitable 32-bit protected mode GDT. You can
  ; check the gdtDescriptor and gdtTable labels for more information regarding our GDT.

  lgdt [gdtDescriptor]

  ; Actually switch into 32-bit protected mode, by setting the PE (Protected Mode Enable) bit
  ; of the CR0 register.
  ; This method isn't compatible with 8086/286, but our code is not designed to be compatible
  ; with systems older than i686 (Pentium Pro / K7 and newer), so it's a non-issue.

  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; Finally, we can jump to our second stage bootloader at 7E00h in memory, setting CS to our
  ; code selector (08h). Hooray! <3

  jmp 08h:7E00h


; -----------------------------------

gdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd gdtTable ; The location of our GDT is the gdtTable label.

gdtTable:

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

; This tells our assembler that we want the rest of our bootsector (essentially any areas that
; haven't been used) to be filled with zeroes, up to the 510th byte.

times 510-($-$$) db 0

; Every x86 bootsector has a signature at the end that the system *always* checks for, which are the
; two bytes 55h and AAh.

; This is required to tell our BIOS that this is a bootsector and not just random noise or data.

db 055h
db 0AAh
