// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"

/* (typedef) struct __attribute__((packed)) realModeTable{}

   Location: (9E00h + E00h) = AC00h

   Inputs:    volatile uint32 Eax-Edx - General purpose registers.
              volatile uint16 Ds-Es - Segment registers.
              volatile uint16 Si-Di - Increment/decrement registers.
              volatile uint16 Bp - Base pointer register.
              uint8 Int - The (BIOS) interrupt we want to call.

   Outputs:   volatile uint32 Eax-Edx - General purpose registers.
              volatile uint16 Ds-Es - Segment registers.
              volatile uint16 Si-Di - Increment/decrement registers.
              volatile uint16 Bp - Base pointer register.
              const volatile uint32 Eflags - The (e)flags register.

   This is a template for the structure containing the registers that will be later loaded (and
   changed) by our real mode functions, in Rm.asm.

   Whenever we go into real mode, this table (which should always be initialized at AC00h) is
   read by our code, and all registers are overwritten with the data in the corresponding fields;
   for example, if you set the Si field to ABCDh and then called realMode(), it would run the
   real mode code with that register set to ABCDh.

   Most of these fields are read by and later written to by our real mode code, but there are
   two exceptions:

    - The Eflags field, which isn't read before executing the program, and which is only written
    to after the fact (you can use it to check for the carry/zero/etc. flag)

    - The Int field, which is only read before executing the program, not written to afterwards;
    it indicates which (BIOS) interrupt you'd like to call in real mode

   Our real mode payload assumes that this struct exists at a specific memory location (AC00h),
   so, before calling realMode(), make sure to initialize this table at that location.

*/

typedef struct {

  // General purpose registers.

  volatile uint32 Eax;
  volatile uint32 Ebx;
  volatile uint32 Ecx;
  volatile uint32 Edx;

  // Segment registers.

  volatile uint16 Ds;
  volatile uint16 Es;

  // Increment/decrement registers.

  volatile uint16 Si;
  volatile uint16 Di;

  // Base pointer register.

  volatile uint16 Bp;

  // Eflags register (output-only).

  const volatile uint32 Eflags;

  // Interrupt that should be called (input-only).

  uint8 Int;

} __attribute__((packed)) realModeTable;


/* void realMode()

   Inputs: (None, except realModeTable at AC00h)
   Outputs: (None, except realModeTable at AC00h)

   This function calls our real mode code, which should be stored at 9E00h. It assumes there's
   an instance of the realModeTable at 9E00h, but if there isn't, it'll just do nothing (assuming
   that memory area is empty).

   This is essentially just a wrapper for the code in Rm.asm, which does the following:

    -> Prepares a 16-bit real mode environment
    -> Loads the data from the realModeTable instance at 9E00h
    -> Calls the interrupt given in realModeTable->Int
    -> Loads data back into the realModeTable instance at 9E00h
    -> Prepares a 32-bit protected mode environment
    -> Returns from the call instruction in this function

*/

void RealMode(void) {

  __asm__("movl $0x9E00, %%eax; call *%%eax" : : : "eax");

}


/* realModeTable* initializeRealModeTable()

   Inputs: (None)
   Outputs: realModeTable* Table - A pointer to an initialized realModeTable struct

   Initializes/clears the realModeTable struct at AC00h, which can then be modified to pass
   parameters/registers to our real mode code (which can be executed using realMode()).

*/

realModeTable* InitializeRealModeTable(void) {

  realModeTable* Table = (void*)0xAC00;

  Table->Eax = 0;
  Table->Ebx = 0;
  Table->Ecx = 0;
  Table->Edx = 0;

  Table->Ds = 0;
  Table->Es = 0;

  Table->Si = 0;
  Table->Di = 0;
  Table->Bp = 0;

  Table->Int = 0;

  return Table;

}
