/* Copyright (C) 2025 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Entrypoint) /* The entrypoint of our kernel. */

SECTIONS
{

  . = 0; /* This is mandatory for the position-independent code to work */

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

}

OUTPUT_FORMAT("elf64-x86-64") /* The output format here is an ELF64 executable */
