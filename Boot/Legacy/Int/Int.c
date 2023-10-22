// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Stdint.h"
#include "../Memory/Memory.h"

// Todo - uhhh, probably everything at this point.
// IDT should be at D000h.
// IDTR should be (2048-1), D000h.

// We should probably try to initialize it as soon as possible, so that we can deal with
// exceptions.

// Also, we need to implement it in Rm.asm when we're coming back from real mode.
