// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Shared/Stdint.h"
#include "../Graphics/Graphics.h"

// The command and data ports for PIC A (sometimes called the master PIC) and PIC B (sometimes
// called the slave PIC) respectively.

#define PicA_Command 0x20
#define PicA_Data 0x21

#define PicB_Command 0xA0
#define PicB_Data 0xA1


/* uint8 Inb()

   Inputs: uint16 Port - The port we want to receive data from.

   Outputs: uint8 - The received data.

   This function essentially just reads data from a certain port - specifically, it *reads* one
   byte (the B in in[b] refers to 'byte') from a given port, and then outputs it.

   In simpler terms, this is essentially just a wrapper for the 'inb' instruction.

*/

uint8 Inb(uint16 Port) {

  uint8 Output = 0;
  __asm__("inb %1, %0" : "=a"(Output) : "Nd"(Port));

  return Output;

}


/* void Outb()

   Inputs: uint16 Port - The port we want to send data to.
           uint8 Data - The data we want to send to that port.

   Outputs: (None)

   This function essentially just sends data to a certain port - specifically, it *sends* one
   byte (the B in out[b] refers to 'byte') to a given port. As we're only sending data (and not
   receiving data), there's no output value like in Inb().

   In simpler terms, this is essentially just a wrapper for the 'outb' instruction.

*/

void Outb(uint16 Port, uint8 Data) {

  __asm__("outb %0, %1" :: "a"(Data), "Nd"(Port));

}


/* static inline void Wait()

   Inputs: (None)
   Outputs: (None)

   This function is just a wrapper for the 'nop' instruction.

*/

static inline void Wait(void) {

  __asm__("nop");

}


// This is an array of strings that essentially indicates which one belongs to each IRQ.
// For example, if you received an IRQ 8, you could look up Irqs[8] and figure out that it's
// the CMOS clock.

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


/* void IrqHandler()

   Inputs: uint8 Vector - The vector number of the IRQ.
           uint8 Port - The port to read from, if necessary (if it isn't, use 00h).

   Outputs: (None)

   This function essentially acts as a generic ISR (interrupt service routine) / interrupt
   handler for an IRQ.

   It just shows a message on screen (using the Message() function from Exceptions.c), and, if
   necessary, reads from a specific port. This is needed for some IRQs, as some of them won't
   work again until the IRQ is acknowledged (what's called an EOI).

   If you don't want your IRQ handler to read from any port, just set it as 00h, and it won't
   do anything.

*/

// [DEMO] MOUSE DATA

volatile uint8 Demo_MouseData[3] = {0, 0, 0};
volatile uint8 Demo_MouseCycleThing = 0;

volatile uint16 Demo_MousePosition[2] = {0, 0};
volatile uint16 Demo_MouseBoundaries[2] = {720, 480};

void Demo_8042Send(uint8 Message);
void Demo_8042Wait(uint8 Type);

void IrqHandler(uint8 Vector, uint8 Port) {

  // [DEMO] - Handle mouse data (IRQ 12)

  // Whenever we get a new mouse movement, we get three separate IRQs for some reason, and
  // this helps keep track of it.

  if (Vector == 12) {

    extern volatile vbeModeInfoBlock* ModeInfo;

    // Get data, and check for sync errors

    Demo_MouseData[Demo_MouseCycleThing] = Inb(0x60);

    if (Demo_MouseCycleThing == 0) {

      if (Demo_MouseData[0] & (1 << 3) == 0) {
        Demo_8042Send(0xFF);
        return;
      }

    }

    Demo_MouseCycleThing++;

    // meow

    if (Demo_MouseCycleThing >= 3) {

      // We've reached the last one, so let's update it

      Demo_MouseCycleThing = 0;

      // Calculate the actual.. position, basically

      uint8 State = Demo_MouseData[0];
      int16 Demo_MouseX = (int16)(Demo_MouseData[1]) - ((State << 4) & 0x100);
      int16 Demo_MouseY = (int16)(Demo_MouseData[2]) - ((State << 3) & 0x100);

      // Un-invert the last position (unless the left-click button is being held)

      extern void Demo_DrawMouse(vbeModeInfoBlock* Info, uint16 Position[2], uint16 Boundaries[2]);
      if (!(State & 1)) Demo_DrawMouse(ModeInfo, Demo_MousePosition, Demo_MouseBoundaries);

      // Update the position variable thing.

      int16 NewX = (Demo_MousePosition[0] + Demo_MouseX) % Demo_MouseBoundaries[0];
      while (NewX < 0) NewX += Demo_MouseBoundaries[0];
      Demo_MousePosition[0] = NewX;

      int16 NewY = (Demo_MousePosition[1] - Demo_MouseY) % Demo_MouseBoundaries[1];
      while (NewY < 0) NewY += Demo_MouseBoundaries[1];
      Demo_MousePosition[1] = NewY;

      // Invert the new position
      Demo_DrawMouse(ModeInfo, Demo_MousePosition, Demo_MouseBoundaries);

    }

    return;

  }


  // Show a message to the user (saying that an IRQ has occured, and which).
  // We take our message from the Irqs[] array.

  Putchar('\n', 0);
  Message(Kernel, "An IRQ has occured.");

  Message(Info, Irqs[Vector]);

  // If necessary, read from a specific port, using the Inb() function.

  if (Port != 0x00) {

    (void)Inb(Port);

  }

}


/* void InitPic()

   Inputs: uint8 PicA_Offset - The 'offset' at which IRQs from PIC-A (also known as the master
           IRQ) should be at.
           uint8 PicB_Offset - The 'offset' at which IRQs from PIC-B (also known as the slave
           IRQ) should be at.

   Outputs: (None)

   This function initializes the 8259 PIC, which is a part of our firmware/motherboard that
   governs certain aspects of our system, like the management of IRQs.

   The PIC dates back to the IBM PC, and although it's been succeded by the APIC, it still
   allows us to gain access to a number of functions, like a built-in timer, and a PS/2 keyboard
   and mouse (if one is available), among others.

   In order to (re-)initialize the PIC, we need to send the following instructions (keep in mind
   that essentially every modern system has two PICs, normally referred to as the master and
   slave PIC, but called PIC-A and PIC-B here):

   -> ICW1 (0x11, 0x11), which indicates to the PIC that we're going to initialize it;
   -> ICW2 (PicA_Offset, PicB_Offset), which indicates the vector offsets for each PIC's IRQs;
   -> ICW3 (0x04, 0x02), which tells the PIC how it's 'wired';
   -> ICW4 (0x01, 0x01), which gives additional information about our system.

*/

void InitPic(uint8 PicA_Offset, uint8 PicB_Offset) {

  // We want to keep the same masks from before, and (re-)initializing the PIC erases them,
  // so we'll save them here.

  uint8 PicA_Mask = Inb(PicA_Data);
  uint8 PicB_Mask = Inb(PicB_Data);

  // Start the initialization process (ICW1).

  Outb(PicA_Command, 0x11); Wait();
  Outb(PicB_Command, 0x11); Wait();

  // Set the vector offset for IRQs from each PIC (ICW2).

  Outb(PicA_Data, PicA_Offset); Wait();
  Outb(PicB_Data, PicB_Offset); Wait();

  // Tell each PIC how it's wired to each other (ICW3).

  Outb(PicA_Data, 4); Wait();
  Outb(PicB_Data, 2); Wait();

  // Tell it to use 8086 mode (ICW4).
  // ..Yes, seriously

  Outb(PicA_Data, 0x01); Wait();
  Outb(PicB_Data, 0x01); Wait();

  // Alright, done! At this point, we've fully initialized both PICs, so we can restore our
  // PIC's old masks.

  Outb(PicA_Data, PicA_Mask);
  Outb(PicB_Data, PicB_Mask);

}


/* void MaskPic()

   Inputs: uint16 Mask - The mask we want to put on our PIC.

   Outputs: (None)

   This function serves one simple, yet important job - it tells the system's PIC(s) to put
   on a "mask".

   PIC masks are important, as they allow for more granular control over which IRQs fire (and
   which ones don't). We don't always want every IRQ to be firing, so this function ends up
   being pretty important.

   There are 16 IRQs, each of which corresponds to a specific bit in the mask. For example, if
   you wanted to un-mask IRQ 0 and IRQ 2, you'd clear those two bits, while setting all the
   other ones.

*/

void MaskPic(uint16 Mask) {

  Outb(PicA_Data, (uint8)(Mask & 0xFF)); // Master PIC (0x21)
  Outb(PicB_Data, (uint8)((Mask >> 8) & 0xFF)); // Slave PIC (0xA1)

}


// [DEMO] Wait for 8042 input

void Demo_8042Wait(uint8 Type) {

  unsigned int Timeout = 100000;

  if (Type == 0) {

    while (Timeout--) {
      if (Inb(0x64) & 1 == 1) return;
    }

  } else {

    while (Timeout--) {
      if (Inb(0x64) & 2 == 0) return;
    }

  }

  return;

}

// [DEMO] Send something to the 8042 controller

void Demo_8042Send(uint8 Message) {

  Demo_8042Wait(1); Outb(0x64, 0xD4); // 0xD4 -> send something
  Demo_8042Wait(1); Outb(0x60, Message);

}
