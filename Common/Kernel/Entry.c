// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

void __attribute__((noreturn)) Entrypoint(unsigned int InfoTable) {

  // For now, infotable is *not* an actual info table, it's just
  // being used to verify what's been passed

  // (in this case it's literally just a pointer to a string)

  unsigned short* Thing = (unsigned short*)0xB8000;
  char* Thing2 = (char*)(InfoTable);

  int Position = 0;

  while (Thing2[Position] != '\0') {

    Thing[Position] = 0x0B00 + Thing2[Position];
    Position++;

  }

  // TODO: Set up environment
  // TODO: Set up basic IDT, panic handling, etc.
  // TODO: Enable MMX, SSE, SSE2.. this should be done in the bootloader, though

  for(;;);

}
