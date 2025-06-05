; Copyright (C) 2025 NunoLealF
; This file is part of the Serra project, which is released under the MIT license.
; For more information, please refer to the accompanying license agreement. <3

[ORG 7FE00h] ; We are at 7FE00h.
[BITS 64] ; This is 64-bit code.

db 0FFh ; (This is a placeholder, so Setup_Int13Wrapper() doesn't see this as empty data for now.)


; ----------------------------------------------------------------------

; (Tell our assembler that we want to pad the rest of our binary with zeroes
; up to the 512th byte (up to 80000h))

times 512 - ($-$$) db 0

; ----------------------------------------------------------------------

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
