# Copyright (C) 2023 NunoLealF
# This file is part of the Serra project, which is released under the MIT license.
# For more information, please refer to the accompanying license agreement. <3

# We want to be able to call these functions from the rest of our bootloader, so we use .globl
# to turn them into global functions.

# .globl ?

# 0 - divide error (DE), fault
# 1 - debug (DB), fault/trap
# 2 - NMI, int
# 3 - break (BP), trap
# 4 - overflow (OF), trap
# 5 - bound exceeded (BR), fault
# 6 - invalid opcode (UD), fault
# 7 - device not available (NM), fault
# 8 - double fault (DF), abort
# 9 - reserved, fault
# 10 - invalid tss (TS), fault
# 11 - segment not present (NP), fault
# 12 - stack segment fault (SS), fault
# 13 - general protection (GP/GPF), fault
# 14 - page fault (PF), fault
# 15 - reserved, ?
# 16 - x87 error (MF), fault
# 17 - align check (AC), fault
# 18 - machine check (MC), abort
# 19 - simd exception (XM), fault
# 20 - virt. exception (VE), fault
# 21 to 31 - reserved, ?
# 32 to 255 - user defined, int
