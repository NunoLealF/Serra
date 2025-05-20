// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Entry.h"

/* entrypointReturnStatus Entrypoint()

   Inputs: commonInfoTable* InfoTable - A pointer to the bootloader-provided
           common information table.

   Outputs: entrypointReturnStatus - If the kernel can't be initialized for
            some reason, we return a status code that will then be
            shown to the user.

   TODO - ...

*/

entrypointReturnStatus Entrypoint(commonInfoTable* InfoTable) {

  // [Make sure the bootloader-provided information tables are valid]

  // Before we do anything else, we want to make sure that the InfoTable
  // we were provided is even valid in the first place - and if not,
  // return with an error code (as in entrypointReturnValue{}).

  // (Check whether the table pointer itself is valid)

  if (InfoTable == NULL) {
    return entrypointTablePointerIsNull;
  }

  // (Check if the table header contains valid values)

  if (InfoTable->Signature != commonInfoTableSignature) {
    return entrypointTableInvalidSignature;
  } else if (InfoTable->Version != commonInfoTableVersion) {
    return entrypointTableInvalidVersion;
  } else if (InfoTable->Size != sizeof(commonInfoTable)) {
    return entrypointTableInvalidSize;
  }

  // (Calculate our own table checksum, and compare it against the table's)

  uint16 Checksum = 0;
  uint16 ChecksumSize = (InfoTable->Size - sizeof(InfoTable->Checksum));
  uint8* RawData = (uint8*)InfoTable;

  for (auto Offset = 0; Offset < ChecksumSize; Offset++) {
    Checksum += RawData[Offset];
  }

  if (InfoTable->Checksum != Checksum) {
    return entrypointTableInvalidChecksum;
  }

  // (Check if the system architecture is valid)

  if (InfoTable->System.Architecture == UnknownArchitecture) {

    return entrypointSystemUnknownArchitecture;

  } else if (InfoTable->System.Architecture == x64Architecture) {

    // (Check whether the value of CR0 is zero)

    if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return entrypointSystemInvalidCpuData;
    }

  } else {

    return entrypointSystemUnsupportedArchitecture;

  }

  // (Check whether ACPI and SMBIOS data is valid, if provided)
  // If it isn't, then we set (.IsSupported = false).

  if (InfoTable->System.Acpi.IsSupported == true) {

    if (InfoTable->System.Acpi.Table.Pointer == NULL) {
      InfoTable->System.Acpi.IsSupported = false;
    }

  }

  if (InfoTable->System.Smbios.IsSupported == true) {

    if (InfoTable->System.Smbios.Table.Pointer == NULL) {
      InfoTable->System.Smbios.IsSupported = false;
    }

  }

  // (Check if the firmware data is valid)

  if (InfoTable->Firmware.Type == UnknownFirmware) {

    return entrypointFirmwareUnsupportedType;

  } else if (InfoTable->Firmware.Type == BiosFirmware) {

    // Make sure that we're on an x64 system, since BIOS isn't supported
    // on any other architecture.

    if (InfoTable->System.Architecture != x64Architecture) {
      return entrypointFirmwareUnsupportedType;
    }

    // (Sanity-check the A20 value given)

    if (InfoTable->Firmware.Bios.A20 >= EnabledByMax) {
      return entrypointFirmwareInvalidBiosData;
    }

    // (Check whether the BIOS (E820) memory map is valid)

    if (InfoTable->Firmware.Bios.Mmap.NumEntries == 0) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.EntrySize < biosMmapEntrySizeWithoutAcpi) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.List.Pointer == NULL) {
      return entrypointFirmwareInvalidMmapData;
    }

    // (Check whether the PAT value given is valid, if supported)
    // If it isn't, then we set (.IsSupported = false).

    if (InfoTable->Firmware.Bios.Pat.IsSupported == true) {
      if (InfoTable->Firmware.Bios.Pat.Value == 0) {
        InfoTable->Firmware.Bios.Pat.IsSupported = false;
      }
    }

    // (Check whether the PCI BIOS table given is valid, if supported)
    // If it isn't, then we set (.IsSupported = false).

    if (InfoTable->Firmware.Bios.PciBios.IsSupported == true) {
      if (InfoTable->Firmware.Bios.PciBios.Table.Pointer == NULL) {
        InfoTable->Firmware.Bios.PciBios.IsSupported = false;
      }
    }

    // (Check whether the VBE information given is valid, if enabled)

    if (InfoTable->Firmware.Bios.Vbe.IsSupported == true) {

      if (InfoTable->Firmware.Bios.Vbe.InfoBlock.Pointer == NULL) {
        return entrypointFirmwareInvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeInfo.Pointer == NULL) {
        return entrypointFirmwareInvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeNumber == 0) {
        return entrypointFirmwareInvalidBiosData;
      }

    }

  } else if (InfoTable->Firmware.Type == EfiFirmware) {

    // (Check whether the EFI image handle and system pointer were
    // both correctly passed on)

    if (InfoTable->Firmware.Efi.ImageHandle.Pointer == NULL) {
      return entrypointFirmwareInvalidEfiData;
    }

    if (InfoTable->Firmware.Efi.SystemTable.Pointer == NULL) {
      return entrypointFirmwareInvalidEfiData;
    }

    // (Check whether the EFI (gBS->GetMemoryMap()) memory map is valid)

    if (InfoTable->Firmware.Efi.Mmap.NumEntries == 0) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.EntrySize < efiMmapEntrySize) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.List.Pointer == NULL) {
      return entrypointFirmwareInvalidMmapData;
    }

    // (Check whether the GOP protocol data is valid, if enabled)

    if (InfoTable->Firmware.Efi.Gop.IsSupported == true) {
      if (InfoTable->Firmware.Efi.Gop.Protocol.Pointer == NULL) {
        return entrypointFirmwareInvalidEfiData;
      }
    }

  } else {

    return entrypointFirmwareUnsupportedType;

  }

  // (Check whether the stack data is valid (page-aligned, non-zero))

  if ((InfoTable->Image.StackTop.Address % 4096) != 0) {
    return entrypointImageMisalignedStack;
  } else if ((InfoTable->Image.StackSize % 4096) != 0) {
    return entrypointImageMisalignedStack;
  }

  if (InfoTable->Image.StackTop.Pointer == NULL) {
    return entrypointImageInvalidStack;
  } else if (InfoTable->Image.StackSize == 0) {
    return entrypointImageInvalidStack;
  }

  // (Check whether the kernel image data is valid)

  if (InfoTable->Image.Type == UnknownImageType) {
    return entrypointImageUnsupportedType;
  } else if (InfoTable->Image.Type >= MaxImageType) {
    return entrypointImageUnsupportedType;
  }

  if (InfoTable->Image.Executable.Entrypoint.Pointer == NULL) {
    return entrypointImageInvalidExecutable;
  } else if (InfoTable->Image.Executable.Header.Pointer == NULL) {
    return entrypointImageInvalidExecutable;
  }

  // (Check whether the 'usable' memory map is valid)

  if (InfoTable->Memory.NumEntries == 0) {
    return entrypointMemoryMapHasNoEntries;
  } else if (InfoTable->Memory.List.Pointer == NULL) {
    return entrypointMemoryMapIsNull;
  }

  // (Check that no entries within the 'usable' memory map overlap)

  usableMmapEntry* UsableMmap = InfoTable->Memory.List.Pointer;

  for (auto Index = 1; Index < InfoTable->Memory.NumEntries; Index++) {

    auto EndOfLastEntry = (UsableMmap[Index - 1].Base + UsableMmap[Index - 1].Limit);
    auto StartOfThisEntry = UsableMmap[Index].Base;

    if (StartOfThisEntry < EndOfLastEntry) {
      return entrypointMemoryMapHasOverlappingEntries;
    }

  }

  // (Sanity-check the value of PreserveUntilOffset)

  usableMmapEntry LastUsableMmapEntry = UsableMmap[InfoTable->Memory.NumEntries - 1];
  uint64 EndOfUsableMemory = (LastUsableMmapEntry.Base + LastUsableMmapEntry.Limit);

  if (EndOfUsableMemory <= InfoTable->Memory.PreserveUntilOffset) {
    return entrypointMemoryInvalidPreserveOffset;
  }

  // (Taking into account PreserveUntilOffset, calculate the amount of
  // memory available to the kernel)

  uint64 UsableMemoryAvailable = 0;

  for (auto Index = 0; Index < InfoTable->Memory.NumEntries; Index++) {

    auto Start = UsableMmap[Index].Base;
    auto End = (Start + UsableMmap[Index].Limit);

    if (End < InfoTable->Memory.PreserveUntilOffset) {
      continue;
    } else if (Start < InfoTable->Memory.PreserveUntilOffset) {
      Start = InfoTable->Memory.PreserveUntilOffset;
    }

    UsableMemoryAvailable += (End - Start + 1);

  }

  // (If the amount of memory available to the kernel is under
  // KernelMemoryLimit, then return)

  if (UsableMemoryAvailable < KernelMemoryLimit) {
    return entrypointMemoryUnderLimit;
  }

  // (Check whether the display data is valid)

  if (InfoTable->Display.Type >= MaxDisplay) {
    return entrypointDisplayUnsupportedType;
  }

  if (InfoTable->Display.Edid.IsSupported == true) {

    // (Check whether the preferred resolution makes any sense)
    // If not, then we set (.IsSupported = false).

    if (InfoTable->Display.Edid.PreferredResolution[0] == 0) {
      InfoTable->Display.Edid.IsSupported = false;
    } else if (InfoTable->Display.Edid.PreferredResolution[1] == 0) {
      InfoTable->Display.Edid.IsSupported = false;
    }

    // (Check whether the EDID table is valid or not)
    // If not, then we set (.IsSupported = false).

    if (InfoTable->Display.Edid.Table.Pointer == NULL) {
      InfoTable->Display.Edid.IsSupported = false;
    }

  }

  if ((InfoTable->Display.Type == VgaDisplay) || (InfoTable->Display.Type == EfiTextDisplay)) {

    // (Check whether the text format is supported)

    if (InfoTable->Display.Text.Format == UnknownFormat) {
      return entrypointDisplayInvalidTextData;
    } else if (InfoTable->Display.Text.Format >= MaxFormat) {
      return entrypointDisplayInvalidTextData;
    }

    // (Check whether the limits seem sane - we need at least 80*25.)

    if (InfoTable->Display.Text.LimitX < 80) {
      return entrypointDisplayTooSmall;
    } else if (InfoTable->Display.Text.LimitY < 25) {
      return entrypointDisplayTooSmall;
    }

    // (Check whether the X and Y coordinates are within limits; if not,
    // set them to zero)

    if (InfoTable->Display.Text.XPos > InfoTable->Display.Text.LimitX) {
      InfoTable->Display.Text.XPos = 0;
    }

    if (InfoTable->Display.Text.YPos > InfoTable->Display.Text.LimitY) {
      InfoTable->Display.Text.YPos = 0;
    }

  }

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    // (Check whether the framebuffer exists or not)

    if (InfoTable->Display.Graphics.Framebuffer.Pointer == NULL) {
      return entrypointDisplayInvalidGraphicsData;
    }

    // (Check whether the limits seem sane - we need at least 640*480.)

    if (InfoTable->Display.Graphics.LimitX < 640) {
      return entrypointDisplayTooSmall;
    } else if (InfoTable->Display.Graphics.LimitY < 480) {
      return entrypointDisplayTooSmall;
    }

    // (Check whether the number of bits per pixel seems sane (between 16
    // and 64 bpp, and must be a multiple of 8)

    if ((InfoTable->Display.Graphics.Bits.PerPixel % 8) != 0) {
      return entrypointDisplayInvalidGraphicsData;
    }

    if (InfoTable->Display.Graphics.Bits.PerPixel < 16) {
      return entrypointDisplayInvalidGraphicsData;
    } else if (InfoTable->Display.Graphics.Bits.PerPixel > 64) {
      return entrypointDisplayInvalidGraphicsData;
    }

    // (Check whether the pitch (number of bytes per scanline) looks sane)

    uint64 MinimumPitch = (InfoTable->Display.Graphics.LimitX * (InfoTable->Display.Graphics.Bits.PerPixel / 8));

    if (InfoTable->Display.Graphics.Pitch < MinimumPitch) {
      return entrypointDisplayInvalidGraphicsData;
    }

    // (Check whether any of the bitmasks overlap each other)

    uint64 Red = InfoTable->Display.Graphics.Bits.RedMask;
    uint64 Green = InfoTable->Display.Graphics.Bits.GreenMask;
    uint64 Blue = InfoTable->Display.Graphics.Bits.BlueMask;

    if ((Red & Green) != 0) {
      return entrypointDisplayInvalidGraphicsData;
    } else if ((Red & Blue) != 0) {
      return entrypointDisplayInvalidGraphicsData;
    } else if ((Green & Blue) != 0) {
      return entrypointDisplayInvalidGraphicsData;
    }

    // (Manually count the number of bits per pixel, and check if it
    // matches InfoTable->Display.Graphics.Bits.PerPixel)

    uint8 Bpp = 0;
    auto Combined = (Red | Green | Blue);

    while (Combined != 0) {
      Bpp++;
      Combined >>= 1;
    }

    if (Bpp > InfoTable->Display.Graphics.Bits.PerPixel) {
      return entrypointDisplayInvalidGraphicsData;
    }

  }

  // (Check whether the disk/filesystem data is valid)

  if (InfoTable->Disk.AccessMethod == UnknownMethod) {
    return entrypointDiskUnsupportedMethod;
  } else if (InfoTable->Disk.AccessMethod >= MaxMethod) {
    return entrypointDiskUnsupportedMethod;
  }

  if (InfoTable->Disk.AccessMethod == Int13Method) {

    // (This method is only valid on systems with BIOS firmware)

    if (InfoTable->Firmware.Type != BiosFirmware) {
      return entrypointDiskUnsupportedMethod;
    }

    // (Check if the drive number is valid (00-0Fh, or 80-8Fh)

    if ((InfoTable->Disk.Int13.DriveNumber > 0x10) && (InfoTable->Disk.Int13.DriveNumber < 0x80)) {
      return entrypointDiskInvalidData;
    } else if (InfoTable->Disk.Int13.DriveNumber > 0x90) {
      return entrypointDiskInvalidData;
    }

    // (Check if EDD data is valid)

    if (InfoTable->Disk.Int13.Edd.IsEnabled == true) {
      if (InfoTable->Disk.Int13.Edd.Table.Pointer == NULL) {
        return entrypointDiskInvalidData;
      }
    }

  } else if (InfoTable->Disk.AccessMethod == EfiFsMethod) {

    // (This method is only valid on systems with EFI firmware)

    if (InfoTable->Firmware.Type != EfiFirmware) {
      return entrypointDiskUnsupportedMethod;
    }

    // (Check for null pointers)

    if (InfoTable->Disk.EfiFs.FileInfo.Pointer == NULL) {
      return entrypointDiskInvalidData;
    } else if (InfoTable->Disk.EfiFs.HandleList.Pointer == NULL) {
      return entrypointDiskInvalidData;
    } else if (InfoTable->Disk.EfiFs.Handle.Pointer == NULL) {
      return entrypointDiskInvalidData;
    } else if (InfoTable->Disk.EfiFs.Protocol.Pointer == NULL) {
      return entrypointDiskInvalidData;
    }

  }




  // [Set up global variables]

  // Now that we've checked that the information table really *is* valid, we
  // can move onto preparing the environment for the kernel, by
  // setting up global variables.

  // At this point, some incorrectly compiled position-independent executables
  // might fail to correctly reference global variables, so we also try
  // to check for that.

  // (Set up EFI variables.)

  if (InfoTable->Firmware.Type == EfiFirmware) {

    InitializeEfiTables(InfoTable->Firmware.Efi.SystemTable.Pointer);

    if (gST != InfoTable->Firmware.Efi.SystemTable.Pointer) {
      return entrypointKernelNotPositionIndependent;
    }

  }




  // [Set up the kernel environment]

  // (TODO: If we're booting from EFI, set up alternate translations? Or
  // anything that might require us to restore environment)

  // (TODO: Set up any essential interfaces, get CPUID data, etc.)

  // (TODO: Set up video interfaces, for text output and such)

  // (TODO: Set up memory management)

  // (TODO: Set up interrupts - for now, just timers)

  // (TODO: Set up drivers)




  // [Call the kernel]

  Kernel(InfoTable);




  // [Restore bootloader status, and return]

  // (TODO: ...)

  return entrypointSuccess;

}
