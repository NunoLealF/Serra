/* Copyright (C) 2024 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Bootloader) /* The first function that should be called in our third-stage bootloader. */

SECTIONS
{

  . = 0x20000; /* Tell our linker that our code will start at 20000h. */

  .text :
  {
    *(.text.Bootloader) /* The start of our third stage bootloader should be the Bootloader() function. */
    *(.text*) /* The rest of our third stage bootloader (excluding non-text/code segments). */
  }

  .data :
  {
    *(.data) /* Our binary's data segment. */
  }

  .rodata :
  {
    *(.rodata) /* Our binary's read only data segment. */
  }

  .bss :
  {
    *(COMMON) /* Our binary's COMMON segment. */
    *(.bss) /* Our binary's bss segment. */
  }

}
