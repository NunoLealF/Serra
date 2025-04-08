/* Copyright (C) 2025 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Entrypoint) /* The entrypoint of our kernel. */

SECTIONS
{

  . = 0xFFFFFFFF80000000; /* Tell our linker that our code will start at -2GiB. */

  /* In order to load our kernel from a 32-bit environment, we first need
  to switch into long mode, and prepare the GDT; this stub does that for us. */

  .pmstub BLOCK(4K) : ALIGN (4K)
  {
      *(.pmstub)
  }

  /* The rest of the kernel goes here: */

  .text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

}
