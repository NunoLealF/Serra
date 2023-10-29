// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

// I/O functions

uint8 Inb(uint16 Port) {

  uint8 Output = 0;

  __asm__("inb %1, %0" : "=a"(Output) : "Nd"(Port));

  return Output;

}

void Outb(uint16 Port, uint8 Data) {

  __asm__("outb %0, %1" :: "a"(Data), "Nd"(Port));

}

// Todo: IRQ related functions (disable/enable/remap PIC)

void DisablePic(void) {

  Outb(0x21, 0xFF); // Master PIC (0x21)
  Outb(0xA1, 0xFF); // Slave PIC (0xA1)

}
