// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Exceptions.h"

// ...

#define PicA_Command 0x20
#define PicA_Data 0x21

#define PicB_Command 0xA0
#define PicB_Data 0xA1

// I/O functions

uint8 Inb(uint16 Port) {

  uint8 Output = 0;

  __asm__("inb %1, %0" : "=a"(Output) : "Nd"(Port));

  return Output;

}

void Outb(uint16 Port, uint8 Data) {

  __asm__("outb %0, %1" :: "a"(Data), "Nd"(Port));

}

static inline void Wait(void) {

  __asm__("nop");

}

// ...

static char* Irqs[] = {

  "Timer (IRQ 0)",
  "PS/2 Keyboard (IRQ 1)",
  "Internal (IRQ 2)",
  "COM2 (IRQ 3)",
  "COM1 (IRQ 4)",
  "LPT2 (IRQ 5)",
  "Floppy disk (IRQ 6)",
  "LPT1 (IRQ 7)",

  "CMOS clock (IRQ 8)",
  "Peripheral port A (IRQ 9)",
  "Peripheral port B (IRQ 10)",
  "Peripheral port C (IRQ 11)",
  "PS/2 Mouse (IRQ 12)",
  "FPU/Coprocessor (IRQ 13)",
  "Primary ATA hard drive (IRQ 14)",
  "Secondary ATA hard drive (IRQ 15)"

};

// irq

void IrqHandler(uint8 Vector, uint8 Port) {

  // ...

  Message(Kernel, "An IRQ has occured.");
  Message(Info, Irqs[Vector]);

  // ... (port has to be >0x00 to do anything)

  if (Port != 0x00) {

    int a = Inb(Port);
    (void)a;

  }

  // ...

  return;

}

// Todo: IRQ related functions (disable/enable/remap PIC)

void MaskPic(uint8 Mask) {

  Outb(PicA_Data, Mask); // Master PIC (0x21)
  Outb(PicB_Data, Mask); // Slave PIC (0x21)

}

void InitPic(uint8 PicA_Offset, uint8 PicB_Offset) {

  // Get masks

  uint8 PicA_Mask = Inb(PicA_Data);
  uint8 PicB_Mask = Inb(PicB_Data);

  // Start init (ICW1)

  Outb(PicA_Command, 0x11); Wait();
  Outb(PicB_Command, 0x11); Wait();

  // Set offset (ICW2)

  Outb(PicA_Data, PicA_Offset); Wait();
  Outb(PicB_Data, PicB_Offset); Wait();

  // Tell it how it's wired (ICW3)

  Outb(PicA_Data, 4); Wait();
  Outb(PicB_Data, 2); Wait();

  // Tell it to use 8086 mode (ICW4)
  // ..Yes, seriously

  Outb(PicA_Data, 0x01); Wait();
  Outb(PicB_Data, 0x01); Wait();

  // Restore masks

  Outb(PicA_Data, PicA_Mask);
  Outb(PicB_Data, PicB_Mask);

}
