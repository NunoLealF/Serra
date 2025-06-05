// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Libraries/Stdint.h"
#include "Disk.h"

// (TODO - Include a function to initialize the disk subsystem (?))

// P.S. - This will 100% be a stage 2 kernel environment thing, for now
// I *technically* don't need anything (other than InitializeCpuFeatures to
// make memory functions faster)

// But if in the future I ever want to implement anything other than what's
// provided by the firmware, I don't have much of a choice, I'll have to
// rely on PCI/PCIe/ACPI/even maybe timing
