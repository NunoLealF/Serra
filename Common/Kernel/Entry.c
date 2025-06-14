// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
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
    return EntrypointTablePointerIsNull;
  }

  // (Check if the table header contains valid values)

  if (InfoTable->Signature != commonInfoTableSignature) {
    return EntrypointTableInvalidSignature;
  } else if (InfoTable->Version != commonInfoTableVersion) {
    return EntrypointTableInvalidVersion;
  } else if (InfoTable->Size != sizeof(commonInfoTable)) {
    return EntrypointTableInvalidSize;
  }

  // (Calculate our own table checksum, and compare it against the table's)

  uint16 Checksum = 0;
  uint16 ChecksumSize = (InfoTable->Size - sizeof(InfoTable->Checksum));
  const uint8* RawInfoTable = (const uint8*)InfoTable;

  for (auto Position = 0; Position < ChecksumSize; Position++) {
    Checksum += RawInfoTable[Position];
  }

  if (InfoTable->Checksum != Checksum) {
    return EntrypointTableInvalidChecksum;
  }

  // (Check if the system architecture is valid)

  if (InfoTable->System.Architecture == SystemArchitecture_Unknown) {

    return EntrypointSystemUnknownArchitecture;

  } else if (InfoTable->System.Architecture == SystemArchitecture_x64) {

    // (Check whether the value of CR0 is zero)

    if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return EntrypointSystemInvalidCpuData;
    }

  } else {

    return EntrypointSystemUnsupportedArchitecture;

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

  if (InfoTable->Firmware.Type == FirmwareType_Unknown) {

    return EntrypointFirmwareUnsupportedType;

  } else if (InfoTable->Firmware.Type == FirmwareType_Bios) {

    // Make sure that we're on an x64 system, since BIOS isn't supported
    // on any other architecture.

    if (InfoTable->System.Architecture != SystemArchitecture_x64) {
      return EntrypointFirmwareUnsupportedType;
    }

    // (Sanity-check the A20 value given)

    if (InfoTable->Firmware.Bios.A20 >= A20_Max) {
      return EntrypointFirmwareInvalidBiosData;
    } else if (InfoTable->Firmware.Bios.A20 == A20_Unknown) {
      return EntrypointFirmwareInvalidBiosData;
    }

    // (Check whether the BIOS (E820) memory map is valid)

    if (InfoTable->Firmware.Bios.Mmap.NumEntries == 0) {
      return EntrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.EntrySize < biosMmapEntrySizeWithoutAcpi) {
      return EntrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.List.Pointer == NULL) {
      return EntrypointFirmwareInvalidMmapData;
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
        return EntrypointFirmwareInvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeInfo.Pointer == NULL) {
        return EntrypointFirmwareInvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeNumber == 0) {
        return EntrypointFirmwareInvalidBiosData;
      }

    }

  } else if (InfoTable->Firmware.Type == FirmwareType_Efi) {

    // (Check whether the EFI image handle and system pointer were
    // both correctly passed on)

    if (InfoTable->Firmware.Efi.ImageHandle.Pointer == NULL) {
      return EntrypointFirmwareInvalidEfiData;
    }

    if (InfoTable->Firmware.Efi.SystemTable.Pointer == NULL) {
      return EntrypointFirmwareInvalidEfiData;
    }

    // (Check whether the EFI (gBS->GetMemoryMap()) memory map is valid)

    if (InfoTable->Firmware.Efi.Mmap.NumEntries == 0) {
      return EntrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.EntrySize < efiMmapEntrySize) {
      return EntrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.List.Pointer == NULL) {
      return EntrypointFirmwareInvalidMmapData;
    }

    // (Check whether the GOP protocol data is valid, if enabled)

    if (InfoTable->Firmware.Efi.Gop.IsSupported == true) {
      if (InfoTable->Firmware.Efi.Gop.Protocol.Pointer == NULL) {
        return EntrypointFirmwareInvalidEfiData;
      }
    }

  } else {

    return EntrypointFirmwareUnsupportedType;

  }

  // (Check whether the stack data is valid (page-aligned, non-zero))

  if ((InfoTable->Image.StackTop.Address % 4096) != 0) {
    return EntrypointImageMisalignedStack;
  } else if ((InfoTable->Image.StackSize % 4096) != 0) {
    return EntrypointImageMisalignedStack;
  }

  if (InfoTable->Image.StackTop.Pointer == NULL) {
    return EntrypointImageInvalidStack;
  } else if (InfoTable->Image.StackSize == 0) {
    return EntrypointImageInvalidStack;
  }

  // (Check whether the kernel image data is valid)

  if (InfoTable->Image.Type == ImageType_Unknown) {
    return EntrypointImageUnsupportedType;
  } else if (InfoTable->Image.Type >= ImageType_Max) {
    return EntrypointImageUnsupportedType;
  }

  if (InfoTable->Image.Executable.Entrypoint.Pointer == NULL) {
    return EntrypointImageInvalidExecutable;
  } else if (InfoTable->Image.Executable.Header.Pointer == NULL) {
    return EntrypointImageInvalidExecutable;
  }

  // (Check whether the 'usable' memory map is valid)

  if (InfoTable->Memory.NumEntries == 0) {
    return EntrypointMemoryMapHasNoEntries;
  } else if (InfoTable->Memory.List.Pointer == NULL) {
    return EntrypointMemoryMapIsNull;
  }

  // (Check that no entries within the 'usable' memory map overlap)

  usableMmapEntry* UsableMmap = InfoTable->Memory.List.Pointer;

  for (auto Index = 1; Index < InfoTable->Memory.NumEntries; Index++) {

    // (Check if any entries overlap)

    auto EndOfLastEntry = (UsableMmap[Index - 1].Base + UsableMmap[Index - 1].Limit);
    auto StartOfThisEntry = UsableMmap[Index].Base;

    if (StartOfThisEntry < EndOfLastEntry) {
      return EntrypointMemoryMapHasOverlappingEntries;
    }

  }

  // (Scan through the usable memory map, calculating the amount available
  // to the kernel, and checking that all entries are aligned to page
  // size boundaries *and* are at least `MmapEntryMemoryLimit` bytes)

  uint64 UsableMemoryAvailable = 0;

  for (auto Index = 0; Index < InfoTable->Memory.NumEntries; Index++) {

    // (Add this entry's size to UsableMemoryAvailable)

    auto Start = UsableMmap[Index].Base;
    auto End = (Start + UsableMmap[Index].Limit);

    UsableMemoryAvailable += (End - Start);

    // (Check if any of the entries are not page aligned)

    if ((Start % SystemPageSize) != 0) {
      return EntrypointMemoryMapNotPageAligned;
    } else if ((End % SystemPageSize) != 0) {
      return EntrypointMemoryMapNotPageAligned;
    }

    // (Check if the entry size is at least `MmapEntryMemoryLimit`)

    if ((End - Start) < MmapEntryMemoryLimit) {
      return EntrypointMemoryMapEntryUnderMinSize;
    }

  }

  // (If the amount of memory available to the kernel is under
  // KernelMemoryLimit, then return)

  if (UsableMemoryAvailable < KernelMemoryLimit) {
    return EntrypointMemoryUnderLimit;
  }

  // (Check whether the display data is valid)

  if (InfoTable->Display.Type >= DisplayType_Max) {
    return EntrypointDisplayUnsupportedType;
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

  if ((InfoTable->Display.Type == DisplayType_Vga) || (InfoTable->Display.Type == DisplayType_EfiText)) {

    // (Check whether the text format is supported)

    if (InfoTable->Display.Text.Format == TextFormat_Unknown) {
      return EntrypointDisplayInvalidTextData;
    } else if (InfoTable->Display.Text.Format >= TextFormat_Max) {
      return EntrypointDisplayInvalidTextData;
    }

    // (Check whether the limits seem sane - we need at least 80*25.)

    if (InfoTable->Display.Text.LimitX < 80) {
      return EntrypointDisplayTooSmall;
    } else if (InfoTable->Display.Text.LimitY < 25) {
      return EntrypointDisplayTooSmall;
    }

    // (Check whether the X and Y coordinates are within limits; if not,
    // set them to zero)

    if (InfoTable->Display.Text.PosX > InfoTable->Display.Text.LimitX) {
      InfoTable->Display.Text.PosX = 0;
    }

    if (InfoTable->Display.Text.PosY > InfoTable->Display.Text.LimitY) {
      InfoTable->Display.Text.PosY = 0;
    }

  }

  if ((InfoTable->Display.Type == DisplayType_Vbe) || (InfoTable->Display.Type == DisplayType_Gop)) {

    // (Check whether the framebuffer exists or not)

    if (InfoTable->Display.Graphics.Framebuffer.Pointer == NULL) {
      return EntrypointDisplayInvalidGraphicsData;
    }

    // (Check whether the limits seem sane - we need at least 640*480.)

    if (InfoTable->Display.Graphics.LimitX < 640) {
      return EntrypointDisplayTooSmall;
    } else if (InfoTable->Display.Graphics.LimitY < 480) {
      return EntrypointDisplayTooSmall;
    }

    // (Check whether the number of bits per pixel seems sane (between 16
    // and 64 bpp, and must be a multiple of 8)

    if ((InfoTable->Display.Graphics.Bits.PerPixel % 8) != 0) {
      return EntrypointDisplayInvalidGraphicsData;
    }

    if (InfoTable->Display.Graphics.Bits.PerPixel < 16) {
      return EntrypointDisplayInvalidGraphicsData;
    } else if (InfoTable->Display.Graphics.Bits.PerPixel > 64) {
      return EntrypointDisplayInvalidGraphicsData;
    }

    // (Check whether the pitch (number of bytes per scanline) looks sane)

    uint64 MinPitch = (InfoTable->Display.Graphics.LimitX * (InfoTable->Display.Graphics.Bits.PerPixel / 8));

    if (InfoTable->Display.Graphics.Pitch < MinPitch) {
      return EntrypointDisplayInvalidGraphicsData;
    }

    // (Check whether any of the bitmasks overlap each other)

    uint64 Red = InfoTable->Display.Graphics.Bits.RedMask;
    uint64 Green = InfoTable->Display.Graphics.Bits.GreenMask;
    uint64 Blue = InfoTable->Display.Graphics.Bits.BlueMask;

    if ((Red & Green) != 0) {
      return EntrypointDisplayInvalidGraphicsData;
    } else if ((Red & Blue) != 0) {
      return EntrypointDisplayInvalidGraphicsData;
    } else if ((Green & Blue) != 0) {
      return EntrypointDisplayInvalidGraphicsData;
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
      return EntrypointDisplayInvalidGraphicsData;
    }

  }

  // (Check whether the disk/filesystem data is valid)

  if (InfoTable->Disk.Method == DiskMethod_Unknown) {
    return EntrypointDiskUnsupportedMethod;
  } else if (InfoTable->Disk.Method >= DiskMethod_Max) {
    return EntrypointDiskUnsupportedMethod;
  }

  if (InfoTable->Disk.Method == DiskMethod_Int13) {

    // (This method is only valid on systems with BIOS firmware)

    if (InfoTable->Firmware.Type != FirmwareType_Bios) {
      return EntrypointDiskUnsupportedMethod;
    }

    // (Check if the drive number is valid (00-0Fh, or 80-8Fh)

    if ((InfoTable->Disk.Int13.DriveNumber >= 0x10) && (InfoTable->Disk.Int13.DriveNumber < 0x80)) {
      return EntrypointDiskInvalidData;
    } else if (InfoTable->Disk.Int13.DriveNumber > 0x90) {
      return EntrypointDiskInvalidData;
    }

    // (Check if EDD data is valid)

    if (InfoTable->Disk.Int13.Edd.IsSupported == true) {

      if (InfoTable->Disk.Int13.Edd.Table.Pointer == NULL) {
        return EntrypointDiskInvalidData;
      } else if (InfoTable->Disk.Int13.Edd.BytesPerSector < 512) {
        return EntrypointDiskInvalidData;
      } else if (InfoTable->Disk.Int13.Edd.NumSectors == 0) {
        return EntrypointDiskInvalidData;
      }

    }

  } else if (InfoTable->Disk.Method == DiskMethod_Efi) {

    // (This method is only valid on systems with EFI firmware)

    if (InfoTable->Firmware.Type != FirmwareType_Efi) {
      return EntrypointDiskUnsupportedMethod;
    }

    // (Check for null pointers)

    if (InfoTable->Disk.Efi.FileInfo.Pointer == NULL) {
      return EntrypointDiskInvalidData;
    } else if (InfoTable->Disk.Efi.Handle.Pointer == NULL) {
      return EntrypointDiskInvalidData;
    } else if (InfoTable->Disk.Efi.Protocol.Pointer == NULL) {
      return EntrypointDiskInvalidData;
    }

  }



  // [Run platform-specific checks and initialize relevant variables]

  // We've managed to validate the bootloader-provided information table,
  // which is great, but we may still need to validate a couple more
  // things, depending on the platform we're compiling for.

  // (TODO: Something like "for the x64 architecture...")

  #if defined(__amd64__) || defined(__x86_64__)

    // (Sanity-check the architecture indicated by the information table)

    if (InfoTable->System.Architecture != SystemArchitecture_x64) {
      return EntrypointSystemUnsupportedArchitecture;
    }

    // (Check that the CPUID instruction is supported)

    if (QueryCpuid(0, 0).Rax == 0) {
      return EntrypointSystemDoesntSupportCpuid;
    }

  #endif




  // [Initialize the kernel environment]

  // Now that we've checked that the information table really *is* valid, we
  // can move onto preparing the environment for the kernel.

  // (At this point, some incorrectly compiled position-independent executables
  // might fail to correctly reference global variables, so we also try
  // to check for that.)


  // [Stage 1] Components that don't rely on other subsystems to function;
  // the order in which these are initialized doesn't matter.

  // (Set up platform-specific constructors)

  #if defined(__amd64__) || defined(__x86_64__)
    InitializeCpuFeatures();
  #endif

  // (Set up firmware-specific constructors)

  if (InfoTable->Firmware.Type == FirmwareType_Efi) {

    InitializeEfiTables(InfoTable, InfoTable->Firmware.Efi.SystemTable.Pointer);

    if (gST != InfoTable->Firmware.Efi.SystemTable.Pointer) {
      return EntrypointNotPositionIndependent;
    } else if (ImageHandle != InfoTable->Firmware.Efi.ImageHandle.Pointer) {
      return EntrypointFirmwareInvalidEfiData;
    }

  }

  // (Set up the memory management subsystem, and make sure that it's
  // working as well)

  if (InitializeMemoryManagementSubsystem(InfoTable->Memory.List.Pointer, InfoTable->Memory.NumEntries) == true) {

    if (MmSubsystemData.IsEnabled == false) {
      return EntrypointCouldntInitializeMm;
    }

    if (VerifyMemoryManagementSubsystem(SystemPageSize / 2) == false) {
      return EntrypointCantManageMemory;
    } else if (VerifyMemoryManagementSubsystem(SystemPageSize) == false) {
      return EntrypointCantManageMemory;
    } else if (VerifyMemoryManagementSubsystem(SystemPageSize * 2) == false) {
      return EntrypointCantManageMemory;
    }

  } else {

    return EntrypointCouldntInitializeMm;

  }

  // (TODO - Initialize the ACPI subsystem)

  // (TODO - Initialize the PCI/PCIe subsystem)


  // [Stage 2] Components that rely on other subsystems to function; the
  // order in which these are initialized *can* matter.

  // (Initialize the graphics subsystem, as well as the built-in bitmap
  // font if possible)

  if (InitializeGraphicsSubsystem(InfoTable) == true) {

    if (GraphicsData.IsSupported == true) {

      if (InitializeBitmapFont() == false) {
        return EntrypointCouldntInitializeFont;
      }

    }

  } else {

    return EntrypointCouldntInitializeGraphics;

  }

  // (Initialize the console subsystem)

  if (InitializeConsoleSubsystem(InfoTable) == false) {
    return EntrypointCouldntInitializeConsole;
  }



  // [Stage 3] Components that rely on the previous subsystems to function.

  // (Initialize the disk and filesystem subsystems)

  if (InitializeDiskSubsystem(InfoTable) == true) {

    // TODO - Make a few test reads.

  } else {

    return EntrypointCouldntInitializeDisk;

  }




  // [Transfer control to the kernel core]

  // Now that we've finished setting up the kernel environment, we can
  // safely call KernelCore().

  KernelCore(InfoTable);




  // [Restore the bootloader state, and exit]

  // (Terminate any subsystems that need to be terminated - this can
  // sometimes be necessary if we need to close any EFI protocols)

  if (TerminateDiskSubsystem() != true) {
    Message(Warning, "Failed to terminate the disk subsystem.");
  }

  // (Inform the bootloader about the updated console position by modifying
  // the relevant fields in `InfoTable`, if necessary)

  if (ConsoleData.IsSupported == true) {

    InfoTable->Display.Text.PosX = ConsoleData.PosX;
    InfoTable->Display.Text.PosY = ConsoleData.PosY;

  }

  // (Transfer control back to the bootloader)

  return EntrypointSuccess;

}
