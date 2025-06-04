// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_GRAPHICS_CONSOLE_VGA_H
#define SERRA_KERNEL_GRAPHICS_CONSOLE_VGA_H

  // Include printing functions from Vga.c

  void Putchar_Vga(const char Character, uint8 Attribute);
  void Print_Vga(const char* String, uint8 Attribute);

#endif
