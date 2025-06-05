// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../../Libraries/Stdint.h"
#include "../../Libraries/String.h"
#include "../Disk.h"
#include "Bios.h"

// (TODO - Include a function to interface with int 13h); this will require
// a real mode stub that's either:

// -> (A) Relocatable, in which case we can compile it as an object (?),
// and then hopefully relocate it somewhere

// -> (B) Fixed-position (70000h?), in which case I'll need to assemble
// it as a raw binary, #embed it, copy it to 70000h at init..
