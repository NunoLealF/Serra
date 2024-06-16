# Copyright (C) 2024 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# We want to be able to call these functions (except for KbdInWait and KbdOutWait) from the rest of
# our bootloader, so we use .globl to turn them into global functions.

.globl EnableKbd_A20
.globl EnableFast_A20


#  void KbdInWait(), void KbdOutWait()
#
#  Inputs:   uint8 (al) - Used to poll the keyboard controller; the initial value is irrelevant.
#  Outputs:  (none)
#
#  These two functions essentially wait for the keyboard controller to give a response saying
#  it's ready to receive/send data.
#
#  They do this by reading port 64h (which returns the 8042 controller's status), and testing for
#  bit 0 and 1 respectively. This is done repeatedly until the controller indicates it's ready,
#  at which point it returns to the function it was called from (using ret).
#

KbdInWait:
  in $0x64, %al
  test $0x01, %al
  jz KbdInWait
  ret

KbdOutWait:
  in $0x64, %al
  test $0x02, %al
  jnz KbdOutWait
  ret


#  void EnableKbd_A20()
#
#  Inputs:   (None)
#  Outputs:  (None)
#
#  This function aims to enable the A20 line via use of the 8042 keyboard controller's CCB
#  (Controller Configuration Byte). It does this by setting bit 1 of the CCB, which usually
#  corresponds to the A20 line.
#
#  First, after disabling interrupts, we need to disable both the first and the second PS/2 ports,
#  to avoid any possible interference. Next, we request the keyboard controller to send us the CCB,
#  and then we OR the 2nd bit (bit 1) as to enable the A20 line.
#
#  Finally, we tell the system we want to set our newly-edited CCB as the current CCB, and we bring
#  the current system state back to normal (enabling the two PS/2 ports, enabling interrupts, etc).

EnableKbd_A20:

  # As we'll be using the C calling convention, push everything to the stack as
  # to not damage the state of the program afterwards.

  pushfl
  pushal

  # Disable interrupts, as to not interfere with anything.

  cli

  # Disable the first PS/2 port (usually the keyboard)

  call KbdOutWait
  mov $0xAD, %al
  out %al, $0x64

  # Disable the second PS/2 port, if it exists

  call KbdOutWait
  mov $0xA7, %al
  out %al, $0x64

  # Tell the 8042 controller that we want the contents of the Controller
  # Configuration Byte

  call KbdOutWait
  mov $0xD0, %al
  out %al, $0x64

  # Wait for it to output the Controller Configuration Byte, and then push
  # it to the stack.

  call KbdInWait
  in $0x60, %al

  # Enable bit 1 of al, which corresponds to the A20 gate, and then push it
  # to the stack so we can use it later.

  or $0b00000010, %al
  push %ax

  # Tell the controller we want to modify the Controller Configuration Byte

  call KbdOutWait
  mov $0xD1, %al
  out %al, $0x64

  # Wait until the 8042 controller is ready to receive the data, and then pop
  # our previously-modified CCB from the stack, sending it to the controller.

  call KbdOutWait
  pop %ax
  out %al, $0x64

  # Re-enable the second PS/2 port, if it exists.

  call KbdOutWait
  mov $0xA8, %al
  out %al, $0x64

  # Re-enable the first PS/2 port, if it exists.

  call KbdOutWait
  mov $0xAE, %al
  out %al, $0x64

  # It's now safe to re-enable interrupts.

  sti

  # Pop the registers we pushed onto the stack initially, and return.

  popal
  popfl

  ret


#  void EnableFastA20()
#
#  Inputs:   (None)
#  Outputs:  (None)
#
#  This function aims to enable the A20 line via use of the 'fast A20' method. It does this by
#  reading a byte from port 92h, enabling bit 1 (which is used for the A20 line), and writing
#  the modified byte back to port 92h.
#
#  Not all systems support this, and this may even cause a crash / undefined behavior. However,
#  this still works on many systems, and given that this is the last method we actually try,
#  it's probably fine tbh.
#

EnableFast_A20:

  # As we'll be using the C calling convention, push everything to the stack as
  # to not damage the state of the program afterwards.

  pushfl
  pushal

  # Disable interrupts as well.

  cli

  # Read the byte from port 92h to al.

  in $0x92, %al

  # Enable bit 1 to tell the system that we want A20 to be on.

  or $0b00000010, %al

  # Write the same byte to port 92h. This may cause undefined behavior, but given that this is
  # the last method we try, it shouldn't matter much.

  out %al, $0x92

  # Enable interrupts.

  sti

  # Pop the registers we pushed onto the stack initially, and return.

  popal
  popfl

  ret
