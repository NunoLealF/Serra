# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# Use .globl functions to make them 'public'

.globl DisablePic

# Todo: IRQ related functions (disable/enable/remap PIC)
# void DisablePic() ...

DisablePic:

  # ...

  pushfl
  pushal

  # ...

  mov $0xFF, %al
  out %al, $0xA1
  out %al, $0x21

  # ...

  popal
  popfl
