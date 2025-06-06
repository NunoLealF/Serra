; Copyright (C) 2025 NunoLealF
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

  times 19h db 0
  BPB.HiddenSectors: dd 0
  times 22h db 0
  BPB.Signature: db 28h

  times 120 - ($-$$) db 0



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

  ; Before we do anything, it's important that we turn off interrupts - not all
  ; segment registers are properly set at the moment!

  cli

  ; Now, we can move on to actually setting up the segment registers (DS, ES
  ; as well as SS), along with the stack.

  ; We set up the stack at 7C00h, as that location should be available on
  ; essentially *all* x86 systems, and because it'll leave the stack with
  ; around 30KiB of space, which should be more than enough.

  ; (Set up the initial segment registers, except for DS)

  SetupRegisters:

  mov ax, 0
  mov es, ax
  mov ss, ax

  mov sp, 7C00h

  ; Next, we need to copy the MBR partition table entry - stored in DS:SI - to
  ; the MbrEntry table here in the bootsector.

  ; (For context: on MBR targets, the current partition table is passed over
  ; in DS:SI; and on MBR-less targets, you can check whether the table is
  ; valid, which we do later on.)

  CopyMbrEntry:

  mov ds, bx
  mov si, cx
  mov di, MbrEntry

  mov cx, 16
  rep movsb

  ; Now that we've copied it, we can safely discard the old value of DS (and
  ; SI), initializing them normally, before saving the value of DL.

  ; (Our BIOS *always* passes the correct drive number in DL, which is why
  ; we need to save it - the MBR also preserves it for us.)

  SetupRemainingRegisters:

  mov ax, 0
  mov ds, ax
  mov si, ax

  mov [SaveDl], dl

  ; Check to see if the MBR entry is valid, and if so, update the BPB to have
  ; the correct amount of hidden sectors; *most partitioning tools don't
  ; update this correctly, for some reason*.

  ; (This may cause issues on disks without an MBR, and it also modifies the
  ; BPB, but the alternative is causing the partition to become completely
  ; unbootable whenever it's moved or resized.)

  VerifyMbrEntry:

  mov al, [MbrEntry.Attributes]
  cmp al, 0x80
  jne UpdatePartitionOffset

  mov eax, [MbrEntry.Lba]
  cmp eax, 0
  je UpdatePartitionOffset

  mov [BPB.HiddenSectors], eax

  ; Since we're in the bootsector of a FAT partition - and not necessarily an
  ; MBR - we can't really assume our bootloader starts at the very start of
  ; the disk.

  ; Because of that, we need to add the number of hidden sectors (which is
  ; a fancy way of saying the LBA offset of the partition) to the offset in
  ; the disk address packet.

  UpdatePartitionOffset:

  mov eax, [DiskAddressPacket.Offset]
  add eax, [BPB.HiddenSectors]
  mov [DiskAddressPacket.Offset], eax

  ; Finally, we can enable interrupts and jump to LoadDisk!

  sti
  jmp LoadDisk



; Now that we've set up a basic environment for our bootloader (segment registers and stack
; have been set up, interrupts are on), we can attempt to load the second stage bootloader.

; We'll be doing this using the BIOS interrupt call int 13h / ah 42h, which is available on
; most systems, and which allows us to load data into memory (sometimes limited to 127 sectors
; at a time) using LBAs.

; Additionally, we'll also try to do this eight times before 'giving up'. This is because some
; older systems may actually require multiple tries in order to successfully load all of the
; sectors.

LoadDisk:

  ; Before we actually try to read anything from disk, we'll need to check to see if the
  ; BIOS function to do so is even available in the first place. We can check this by calling
  ; the function (int 13h / ah 41h).

  mov ah, 41h
  mov bx, 55AAh

  int 13h

  ; If the carry flag is set, ah is set to 80h (invalid command) or 86h (unsupported function),
  ; then jump to the showError label, as that indicates the disk read function isn't supported.

  jc ShowError

  cmp ah, 80h
  je ShowError

  cmp ah, 86h
  je ShowError

  ; Now, we can finally use the BIOS interrupt call (int 13h / ah 42h) to load the second-stage
  ; bootloader, using the data in [DS:SI] (which in this case, points to the data in the
  ; diskAddressPacket label). As always, we use the drive number in SaveDl.

  mov ah, 42h
  mov dl, [SaveDl]

  mov si, DiskAddressPacket

  int 13h

  ; At this moment, there are two possibilities - either the data was loaded successfully
  ; (and it's safe to proceed to the next stage), or it failed to load properly. This is
  ; indicated by the carry flag.

  ; If the data was loaded successfully, we'll proceed to the protectedMode label, which
  ; should initialize 32-bit protected mode and then jump to 7E00h - otherwise, we'll jump
  ; to the showError label, which should show an error and halt the system.

  jnc ProtectedMode
  jmp ShowError



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

ProtectedMode:

  ; Try to enable A20 via the int 15h, ax 2401h BIOS interrupt. Not guaranteed to work, but
  ; it might, so..

  mov ax, 2401h
  int 15h

  ; As we'll be messing with the GDT and enabling protected mode, we'll definitely need to
  ; disable interrupts to prevent things from going south.

  cli

  ; Load [GdtDescriptor], which should contain a suitable 32-bit protected mode GDT. You can
  ; check the GdtDescriptor and gdtTable labels for more information regarding our GDT.

  lgdt [GdtDescriptor]

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



; If for some reason we fail to load the rest of the bootloader, then we should call this
; function, to halt the system, but also to show an error message to the user.

ShowError:

  ; Show an error message using the printMessage function. [DS:SI] is our string, [ES:BX] is
  ; the buffer to display the text in (in this case, B800:0000h), and CL is our color code.

  mov ax, 0B800h
  mov bx, 0h
  mov cl, 0Fh

  mov es, ax
  mov si, errorMsg

  call PrintMessage

  ; Finally, now that we've shown the message, disable all interrupts, and halt the system.

  cli
  hlt



; This function prints a string (in DS:SI) to the screen (whose buffer should be in ES:BX),
; using a certain color code (CL).

PrintMessage:

  ; Load the value at [DS:SI] into the AL register.

  lodsb

  ; If AL (the character we just loaded from the string) is 0, then that means we've
  ; reached the end, so let's return from this function.

  cmp al, 0
  je PrintMessageRet

  ; Copy AL (the character) and CL (the color code) respectively to the location in [ES:BX],
  ; and then increment bx (to continue writing to the next character on the screen).

  mov [es:bx], al
  inc bx
  mov [es:bx], cl
  inc bx

  ; Jump back to PrintMessage until we're done, pretty much.

  jmp PrintMessage
  PrintMessageRet: ret


; -----------------------------------

errorMsg db "[Serra] Failed to load the rest of the bootloader.", 0

; -----------------------------------

MbrEntry:
  .Attributes: db 0
  .ChsStart.High: db 0
  .ChsStart.Low: dw 0

  .Type: db 0
  .ChsEnd.High: db 0
  .ChsEnd.Low: dw 0

  .Lba: dd 0
  .NumSectors: dd 0

; -----------------------------------

DiskAddressPacket:

  .Size: db 16 ; The size of this (disk address) packet is 16 bytes.
  .Reserved: db 0 ; (This area is reserved)
  .NumSectors: dw 24 ; We want to load 24 sectors into memory.
  .Location.Offset: dw 7E00h ; And we also want to load those at 0000:[7E00h].
  .Location.Segment: dw 0000h ; And we also want to load those at [0000]:7E00h.
  .Offset: dq 16 ; Additionally, we want to start loading from LBA 16 (this value may be changed).

; -----------------------------------

GdtDescriptor:

  dw 24 - 1 ; Three eight-byte segments, minus one byte.
  dd GdtTable ; The location of our GDT is the gdtTable label.

GdtTable:

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

; Tell our assembler that we want to fill the rest of our bootsector (basically,
; (any areas that haven't been used) with zero up to the (512 - 20)th byte, to
; account for Debug, SaveDl and BootsectorSignature.

times (512 - 4) - ($-$$) db 0

; We want to have an easy way to toggle non-debug messages off and on, so we'll reserve a
; byte here in the bootsector for exactly that - in order to toggle *on* messages, set this
; to anything but 00h.

Debug: db 01h

; This is where we'll reserve the drive number given to us by our BIOS, which is in the DL
; register - this will be important later on. By hardcoding the position (as being immediately
; before the bootsector signature, AA55h), we know exactly where to find it in the future.

SaveDl: db 00h

; Every x86 bootsector has a signature at the end that the system *always* checks for, which are the
; two bytes 55h and AAh (AA55h). This is required if we want to tell our BIOS that this is a
; bootsector, and not just random noise or data.

BootsectorSignature: dw 0AA55h
