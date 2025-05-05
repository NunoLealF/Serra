; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

%define RelocatedMbrAddress 600h
%define BootsectorAddress 7C00h

[ORG RelocatedMbrAddress] ; We are at RelocatedMbrAddress.. sort of.
[BITS 16] ; This is 16-bit code.

Start:

  ; Before anything, we should probably reset the code segment to 0,
  ; which can be done with a far jump.

  ; (We are *not* at RelocatedMbrAddress, so in order to get the correct
  ; address, we need to tell the assembler to do this instead)

  jmp 00h:(BootsectorAddress + (InitEnvironment - Start))


; As with anything else loaded by the BIOS, our MBR is loaded at 7C00h in
; memory. Usually, this would be fine, but since our bootsector *also*
; expects to be loaded there, we need to do relocate ourselves.

; Thankfully, this is relatively simple: all we really need to do is to move
; 512 bytes (using something like `rep movsb`) from our current position
; (which is 7C00h) somewhere else, and then jump there.

; (We also need to preserve the dl register throughout the entire MBR, so
; we can safely pass it onto the bootsector; this is because it contains
; our current drive number.)

InitEnvironment:

  ; First, let's set up *just* the segment registers, as well as the
  ; stack; we already set the CS register earlier on in Start.

  cli

  mov ax, 0
  mov ds, ax
  mov es, ax
  mov ss, ax

  mov sp, 7C00h

  ; Now, let's relocate ourselves. The first step is relatively easy: we just
  ; need to copy the MBR (which is where we are now) somewhere else.

  ; In this case, we'll be using `rep movsb`, which takes the source address
  ; in DS:SI, the destination address in ES:DI, and the number of bytes to
  ; copy in the CX register. We already set up DS and ES, so:

  mov cx, 512

  mov si, 7C00h
  mov di, RelocatedMbrAddress

  cld
  rep movsb

  ; Now that we've copied ourselves to RelocatedMbrAddress, we just need to
  ; jump to it. This is kind of tricky, but since we know *exactly* where
  ; we are, we can just have the assembler do it for us:

  sti
  jmp 00h:CheckPartitionTables


; Now that we've relocated ourselves to a location that won't be overwritten,
; we can proceed with the next step, which is reading the partition tables.

; More specifically, we need to find a partition table that's marked as
; active/bootable, which we can check by looking at the attributes.

CheckPartitionTables:

  ; First, let's save the value of DL (which contains the drive number),
  ; just in case:

  mov [SaveDl], dl

  ; The active/bootable flag is bit 7, which is 80h in hex, so in order to
  ; see if a given partition is bootable, we just need to AND it with 80h.

  ; (Check if the 1st partition is bootable, and if so, load it)

  .CheckFirstPartition:

    mov bx, 0

    mov al, [FirstPartition.Attributes]
    and al, 80h
    cmp al, 0

    je .CheckSecondPartition
    call LoadPartition

  ; (Check if the 2nd partition is bootable, and if so, load it)

  .CheckSecondPartition:

    mov bx, 1

    mov al, [SecondPartition.Attributes]
    and al, 80h
    cmp al, 0

    je .CheckThirdPartition
    call LoadPartition

  ; (Check if the 3rd partition is bootable, and if so, load it)

  .CheckThirdPartition:

    mov bx, 2

    mov al, [ThirdPartition.Attributes]
    and al, 80h
    cmp al, 0

    je .CheckFourthPartition
    call LoadPartition

  ; (Check if the 4th partition is bootable, and if so, load it)

  .CheckFourthPartition:

    mov bx, 3

    mov al, [FourthPartition.Attributes]
    and al, 80h
    cmp al, 0

    je Error
    call LoadPartition


; If we've gotten here, then that means we want to load a given partition;
; in this case, it's assumed that BX contains (the partition number - 1).

LoadPartition:

  ; Get the location of the partition table itself.
  ; (Start of partition tables + (BX << 4))

  pusha
  sal bx, 4

  mov bp, FirstPartition
  add bp, bx

  ; Check the LBA and CHS values, and if they appear to be correct, load
  ; the partition bootsector.

  ; (Warning: CHS boot is mostly untested; this may not work!)

  .CheckLba:

    ; (Are the LBA values correct?)

    mov ax, [bp + 8]
    mov cx, [bp + 10]

    mov bx, ax
    or bx, cx

    cmp bx, 0
    je .CheckChs

    ; (If so, load the bootsector of that partition, using the extended/LBA
    ; int 13h disk read function)

    pusha
    mov [DiskAddressPacket.Lba.High], ax
    mov [DiskAddressPacket.Lba.Low], cx

    mov ah, 42h
    mov si, DiskAddressPacket
    mov dl, [SaveDl]

    int 13h
    jnc .VerifyBootsector

    popa
    jmp .CheckChs

  .CheckChs:

    ; (Are the CHS values correct, at least?)

    mov ch, [bp + 1]
    mov cl, [bp + 2]
    mov dh, [bp + 3]

    mov bh, ch
    or bh, cl
    or bh, dh

    cmp ch, 0
    je .PartitionTableIsInvalid

    ; (If so, load the bootsector of that partition)

    mov ah, 02h
    mov al, 01h
    mov bx, 7C00h
    mov dl, [SaveDl]

    int 13h
    jmp .VerifyBootsector

  ; If we've gotten here, then that means we've loaded the bootsector, so
  ; we just need to verify it.

  .VerifyBootsector:

    ; (Check the bootloader signature)

    mov ax, [7DFEh] ; Location of the bootloader signature
    cmp ax, 0AA55h

    jne .PartitionTableIsInvalid

    ; (If it's fine, then point DS:SI to the partition table - which is
    ; thankfully already in BP - and jump, preserving DL as well)

    mov ax, 0
    mov ds, ax
    mov si, bp

    mov dl, [SaveDl]

    jmp 00h:BootsectorAddress

  ; If we've gotten here, then the partition table is invalid *or* we
  ; couldn't load it for some reason, so, return.

  .PartitionTableIsInvalid:
    popa
    ret


; If we're here, then that means there's an error of some sort, so we just
; print an error indicator on the screen, before halting.

; (Interrupts are still enabled, so Ctrl+Alt+Delete can be used)

Error:

  ; (Print a red '!' on the screen, using BIOS calls)

  sti

  mov ah, 09h
  mov al, '!'

  mov bx, 0Ch
  mov cx, 1

  int 10h
  jmp .Halt

  ; (Halt indefinitely)

  .Halt:
    hlt
    jmp short .Halt

; ---------------------------------------------------------------------------

SaveDl: db 0

DiskAddressPacket:

  .Size: db 16 ; The size of this table is 16 bytes.

  .Reserved: db 0 ; This area is reserved.
  .NumSectors: dw 1 ; We only want to load a single sector.
  .Location: dd 7C00h ; And we want to load it at 7C00h.

  .Lba.High: dw 0 ; The higher 16-bits of the 32-bit LBA.
  .Lba.Low: dw 0 ; The lower 16-bits of the 32-bit LBA.

  .LbaPadding: dd 0 ; The 32-bits of the LBA we aren't going to use

; ---------------------------------------------------------------------------

; Tell our assembler that we want to fill the rest of our MBR with zeroes, up
; to the 440th byte (which is the end of our MBR 'bootstrap' code).

times 440 - ($-$$) db 0

; Next, let's define the fields used in the MBR. We will *not* be defining
; these - that job is for the actual partitioning tools themselves - but we
; will have to *read* it.

; (Some MBRs will have a disk signature of some sort on bytes 440-446)

DiskSignature: dd 0
Reserved: dw 0

; (Each MBR has 4 partition table entries on bytes 446-510, which are all
; structured the same way, like this)

FirstPartition:

  .Attributes: db 0
  .ChsStart.High: db 0
  .ChsStart.Low: dw 0

  .Type: db 0
  .ChsEnd.High: db 0
  .ChsEnd.Low: dw 0

  .Lba: dd 0
  .NumSectors: dd 0

SecondPartition:

  .Attributes: db 0
  .ChsStart.High: db 0
  .ChsStart.Low: dw 0

  .Type: db 0
  .ChsEnd.High: db 0
  .ChsEnd.Low: dw 0

  .Lba: dd 0
  .NumSectors: dd 0

ThirdPartition:

  .Attributes: db 0
  .ChsStart.High: db 0
  .ChsStart.Low: dw 0

  .Type: db 0
  .ChsEnd.High: db 0
  .ChsEnd.Low: dw 0

  .Lba: dd 0
  .NumSectors: dd 0

FourthPartition:

  .Attributes: db 0
  .ChsStart.High: db 0
  .ChsStart.Low: dw 0

  .Type: db 0
  .ChsEnd.High: db 0
  .ChsEnd.Low: dw 0

  .Lba: dd 0
  .NumSectors: dd 0

; (Finally, as with anything else loaded by the BIOS, the last two bytes
; must always be AA55h)

BootsectorSignature: dw 0AA55h
