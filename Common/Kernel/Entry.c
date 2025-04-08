// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

void __attribute__((noreturn)) Entrypoint(void) {

  *(unsigned short*)(0xB8000) = 0xFFFF;
  for(;;);

}
