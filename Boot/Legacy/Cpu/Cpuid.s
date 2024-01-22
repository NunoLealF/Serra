# Copyright (C) 2024 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

.globl NAME_OF_FUNCTION_HERE

# (...)
# Todo - add a function to check for the CPUID bit in eflags
# https://wiki.osdev.org/CPUID

NAME_OF_FUNCTION_HERE:

  # As we'll be using the C calling convention, push everything to the stack as
  # to not damage the state of the program afterwards.

  pushfl
  pushal

  # (...)

  # Pop the registers we pushed onto the stack initially, and return.

  popal
  popfl

  ret
