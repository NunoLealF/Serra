/* Copyright (C) 2024 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Init) /* The first function that should be called in our second stage bootloader. */

SECTIONS
{

  . = 0x7E00; /* Tell our linker that our code will start at 7E00h. */

  .text :
  {
    *(.text.Init) /* The start of our second stage bootloader should be the Init() function. */
    *(.text*) /* The rest of our second stage bootloader (excluding non-text/code segments). */
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
