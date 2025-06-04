// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_KERNEL_GRAPHICS_CONSOLE_EFI_H
#define SERRA_KERNEL_GRAPHICS_CONSOLE_EFI_H

  // Include printing functions from Efi.c

  void Putchar_Efi(const char Character, int32 Attribute);
  void Print_Efi(const char* String, int32 Attribute);

#endif
