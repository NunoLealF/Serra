/* Copyright (C) 2025 NunoLealF */
/* This file is part of the Serra project, which is released under the MIT license. */
/* For more information, please refer to the accompanying license agreement. <3 */

ENTRY(Entrypoint) /* The entrypoint of our kernel. */

TARGET(binary) /* Directly add a binary blob as an ELF section */
INPUT(./Kernel/Stub.bin) /* (The blob is Kernel/Stub.bin) */

SECTIONS
{

  . = 0xFFFFFFFF80000000; /* Tell our linker that our code will start at -2GiB. */

  /* The sections of the kernel; the protected mode stub is placed after
     .text, whereas the long mode entrypoint is placed at the beginning */

  .text : ALIGN(4K)
	{
    *(.text.Entrypoint)
    *(.text)
	}

  .pmstub (READONLY) : ALIGN(4K)
  {
    KEEP(./Kernel/Stub.bin)
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
