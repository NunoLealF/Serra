/* Copyright (C) 2025 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Entrypoint) /* The entrypoint of our kernel. */

SECTIONS
{

  /* (Regular executable sections; these should all be page-aligned) */

  .text : ALIGN(4K)
	{
    *(.text.Entrypoint)
    *(.text)
	}

	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}

	.data : ALIGN(4K)
	{
		*(.data)
	}

	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

  /* As we're compiling a position-independent executable, we also have a
  `.dynamic` section, that needs to be page-aligned as with any other */

  .dynamic : ALIGN(4K)
  {
    *(.dynamic)
  }

}

OUTPUT_FORMAT("elf64-x86-64") /* The output format here is an ELF64 executable */
