/* Copyright (C) 2025 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Entrypoint) /* The entrypoint of our kernel. */

SECTIONS
{

  /* (Regular executable sections; these should all be page-aligned) */

  .text : ALIGN(4K)
	{
    KEEP(*(.text.Entrypoint .ltext.Entrypoint))
    *(.text* .ltext*)
	}

	.rodata : ALIGN(4K)
	{
		*(.rodata* .lrodata*)
	}

	.data : ALIGN(4K)
	{
		*(.data* .ldata*)
	}

	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss* .lbss*)
	}

  /* As we're compiling a position-independent executable, we also have a
  `.dynamic` section that needs to be page-aligned. */

  .dynamic : ALIGN(4K)
  {
    *(.dynamic*)
  }

  /* Additionally, in order to avoid issues with some linkers (lld), we
  also need to create a separate page-aligned section for `.got.plt`. */

  /* (This is necessary because lld is really fucking stupid.) */

  .got : ALIGN(4K)
  {
    *(.got.plt)
    *(.got*)
  }

  /* Finally, we can also safely discard some unused sections. */

  /DISCARD/ : {
    *(.comment*)
    *(.note*)
  }

}

OUTPUT_FORMAT("elf64-x86-64") /* The output format here is an ELF64 executable */
