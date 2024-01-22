# Copyright (C) 2024 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

.globl CheckCpuid


# uint32 CheckCpuid()
#
# Inputs:   (None)
#
# Outputs:  uint32 - The difference between the value of the EFLAGS register between trying to
#           modify the CPUID bit. (Supports CPUID, >0; otherwise, =0)
#
# This function checks for the presence of CPUID, by testing bit 21 of the EFLAGS register.
#
# With a few exceptions (mainly just some old Cyrix and NexGen CPUs), if the CPUID bit of the
# EFLAGS register can be modified, then that means that the system supports CPUID.
#
# This function takes advantage of that, and tries to check if the CPUID bit of the EFLAGS
# register is modifiable (if it is, then that means CPUID is available). If it is, then it
# should return a positive value, but otherwise, it should return 0.

CheckCpuid:

  # As we'll be using the C calling convention, push everything to the stack as
  # to not damage the state of the program afterwards.

  pushfl
  pushal

  # Use the 'pushfl' instruction to push the current value of the EFLAGS register onto the
  # stack, and store its value in %eax and %ebx.

  pushfl
  pop %eax
  mov %eax, %ebx

  # Using the 'xor' instruction, invert the CPUID bit in %eax (bit 21), and using the 'popfl'
  # instruction, use %eax as the value of the EFLAGS register.

  xor CpuidMask, %eax
  push %eax
  popfl

  # Again, use the 'pushfl' instruction to push the new value of the EFLAGS register onto
  # the stack, and pop it into %eax.

  pushfl
  pop %eax

  # Isolate the CPUID bit from both %eax (new EFLAGS) and %ebx (old EFLAGS), and use the 'xor'
  # instruction on the two, with the resulting value in %eax.

  and CpuidMask, %eax
  and CpuidMask, %ebx

  xor %ebx, %eax

  # We want to preserve %eax ahead of the incoming 'popal' instruction, so we'll store it in
  # [StoreEax].

  mov %eax, StoreEax

  # Pop the registers we pushed onto the stack initially, restore the value of %eax from
  # [StoreEax], and finally return. In this case, %eax is the return value.

  popal
  popfl

  mov StoreEax, %eax

  ret


# A mask that corresponds to bit 21 of the EFLAGS register, which corresponds to the CPUID bit.
# This bit can only be modified if CPUID is on, which is why it's used in CheckCpuid.

CpuidMask:
  .long 0b00000000001000000000000000000000

# A memory area that temporarily stores the value of the %eax register.

StoreEax:
  .long 0
