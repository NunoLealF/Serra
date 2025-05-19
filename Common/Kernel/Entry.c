// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Kernel.h"

/* uint64 Entrypoint()

   Inputs: commonInfoTable* InfoTable - A pointer to the bootloader-provided
           common information table.

   Outputs: entrypointReturnValue - If the kernel can't be initialized for
            some reason, we return a status code that will then be
            shown to the user.

   TODO - ...

*/

entrypointReturnValue Entrypoint(commonInfoTable* InfoTable) {

  // [Make sure the bootloader-provided information tables are valid]

  // Before we do anything else, we want to make sure that the InfoTable
  // we were provided is even valid in the first place - and if not,
  // return with an error code (as in entrypointReturnValue{}).

  // TODO -> Not all of these actually require us to return...                      (TODO TODO TODO TODO)

  // (Check whether the table pointer itself is valid)

  if (InfoTable == NULL) {
    return entrypointTableIsNull;
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

    // (Check whether CR0 and CR3 both aren't zero)

    if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return entrypointSystemInvalidCpuData;
    } else if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return entrypointSystemInvalidCpuData;
    }

  } else {

    return entrypointSystemUnsupportedArchitecture;

  }

  // (Check whether ACPI and SMBIOS data is valid, if provided)

  if (InfoTable->System.Acpi.IsSupported == true) {

    if (InfoTable->System.Acpi.Table.Pointer == NULL) {
      return entrypointSystemInvalidAcpiData;
    }

  }

  if (InfoTable->System.Smbios.IsSupported == true) {

    if (InfoTable->System.Smbios.Table.Pointer == NULL) {
      return entrypointSystemInvalidSmbiosData;
    }

  }

  // (Check if the firmware data is valid)

  if (InfoTable->Firmware.Type == UnknownFirmware) {

    return entrypointFirmwareUnknownType;

  } else if (InfoTable->Firmware.Type == BiosFirmware) {

    // (BIOS is only supported on x86 systems)

    if (InfoTable->System.Architecture != x64Architecture) {
      return entrypointFirmwareUnsupportedType;
    }

    // (Sanity-check the A20 value given)

    if (InfoTable->Firmware.Bios.A20 >= EnabledByMax) {
      return entrypointFirmwareInvalidBiosData;
    }

    // (Check whether the BIOS (E820) memory map is valid)

    #define biosMmapEntrySize 24 // (TODO - there should be something like a biosMmapEntry{} definition)

    if (InfoTable->Firmware.Bios.Mmap.NumEntries == 0) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.EntrySize < biosMmapEntrySize) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.List.Pointer == NULL) {
      return entrypointFirmwareInvalidMmapData;
    }

    // (Check whether the PAT value given is valid, if supported)

    if (InfoTable->Firmware.Bios.Pat.IsSupported == true) {
      if (InfoTable->Firmware.Bios.Pat.Value == 0) {
        return entrypointFirmwareInvalidBiosData;
      }
    }

    // (Check whether the PCI BIOS table given is valid, if supported)

    if (InfoTable->Firmware.Bios.PciBios.IsSupported == true) {
      if (InfoTable->Firmware.Bios.PciBios.Table.Pointer == NULL) {
        return entrypointFirmwareInvalidBiosData;
      }
    }

    // (Check whether the VBE information given is valid, if supported)

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

    #define efiMmapEntrySize sizeof(efiMemoryDescriptor)

    if (InfoTable->Firmware.Efi.Mmap.NumEntries == 0) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.EntrySize < efiMmapEntrySize) {
      return entrypointFirmwareInvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.List.Pointer == NULL) {
      return entrypointFirmwareInvalidMmapData;
    }

    // (Check whether the GOP protocol data is valid, if supported)

    if (InfoTable->Firmware.Efi.Gop.IsSupported == true) {
      if (InfoTable->Firmware.Efi.Gop.Protocol.Pointer == NULL) {
        return entrypointFirmwareInvalidEfiData;
      }
    }

  } else {

    return entrypointFirmwareUnsupportedType;

  }

  // (Check whether the stack data is valid)

  if ((InfoTable->Image.StackTop.Address % 4096) != 0) {
    return entrypointImageInvalidStack;
  } else if ((InfoTable->Image.StackSize % 4096) != 0) {
    return entrypointImageInvalidStack;
  }

  if (InfoTable->Image.StackTop.Pointer == NULL) {
    return entrypointImageInvalidStack;
  } else if (InfoTable->Image.StackSize == 0) {
    return entrypointImageInvalidStack;
  }

  // (Check whether the kernel image data is valid)

  if (InfoTable->Image.Type == UnknownImageType) {
    return entrypointImageUnknownType;
  } else if (InfoTable->Image.Type >= MaxImageType) {
    return entrypointImageUnsupportedType;
  }

  if (InfoTable->Image.Executable.Entrypoint.Pointer == NULL) {
    return entrypointImageInvalidData;
  } else if (InfoTable->Image.Executable.Header.Pointer == NULL) {
    return entrypointImageInvalidData;
  }

  // (Check whether the 'usable' / kernel memory map is valid)

  if (InfoTable->Memory.NumEntries == 0) {
    return entrypointMemoryListHasNoEntries;
  } else if (InfoTable->Memory.List.Pointer == NULL) {
    return entrypointMemoryListIsNull;
  }

  // (Check whether the display data is valid)

  if (InfoTable->Display.Type >= MaxDisplay) {
    return entrypointDisplayUnsupportedType;
  }

  if (InfoTable->Display.Edid.IsSupported == true) {

    // (Check whether the preferred resolution makes any sense)

    if (InfoTable->Display.Edid.PreferredResolution[0] == 0) {
      return entrypointDisplayInvalidEdidData;
    } else if (InfoTable->Display.Edid.PreferredResolution[1] == 0) {
      return entrypointDisplayInvalidEdidData;
    }

    // (Check whether the EDID table is valid or not)

    if (InfoTable->Display.Edid.Table.Pointer == NULL) {
      return entrypointDisplayInvalidEdidData;
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
      return entrypointDisplayInvalidTextData;
    } else if (InfoTable->Display.Text.LimitY < 25) {
      return entrypointDisplayInvalidTextData;
    }

  }

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    // (Check whether the framebuffer exists or not)

    if (InfoTable->Display.Graphics.Framebuffer.Pointer == NULL) {
      return entrypointDisplayInvalidGraphicsData;
    }

    // (Check whether the limits seem sane - we need at least 640*480.)

    if (InfoTable->Display.Graphics.LimitX < 640) {
      return entrypointDisplayInvalidGraphicsData;
    } else if (InfoTable->Display.Graphics.LimitY < 480) {
      return entrypointDisplayInvalidGraphicsData;
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
    return entrypointDiskUnknownMethod;
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

  // Now that we've checked that the information table really *is* valid,
  // we can move onto preparing the environment for the kernel, by
  // setting up global variables.

  if (InfoTable->Firmware.Type == EfiFirmware) {

    gST = InfoTable->Firmware.Efi.SystemTable.Pointer;
    gBS = gST->BootServices;
    gRT = gST->RuntimeServices;

  }



  // [Demo]

  if (InfoTable->Firmware.Type == EfiFirmware) {

    __asm__ __volatile__ ("sti");

    if (InfoTable->Display.Type == EfiTextDisplay) {

      if (InfoTable->Firmware.Efi.SupportsConOut == true) {

        gST->ConOut->ClearScreen(gST->ConOut);

        gST->ConOut->SetAttribute(gST->ConOut, 0x0F);
        gST->ConOut->OutputString(gST->ConOut, u"Hi, this is Serra! <3 \n\r");

        gST->ConOut->SetAttribute(gST->ConOut, 0x3F);
        gST->ConOut->OutputString(gST->ConOut, u"May 19 2025");

        gST->ConOut->SetAttribute(gST->ConOut, 0x07);
        gST->ConOut->OutputString(gST->ConOut, u"\n\r");

      }

    } else {

      for (auto y = 0; y < InfoTable->Display.Graphics.LimitY; y++) {

        for (auto x = 0; x < (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 4); x++) {

          *(uint8*)(InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * y) + x) = (y * 255 / InfoTable->Display.Graphics.LimitY);

        }

      }

    }

    efiInputKey PhantomKey;

    if (InfoTable->Firmware.Efi.SupportsConIn == false) {
      while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);
    }

    return entrypointSuccess;

  } else {

    if (InfoTable->Display.Type == VgaDisplay) {

      char* Test = "Hi, this is Serra! <3";
      char* Test2 = "May 19 2025";

      uint16* Terminal = (uint16*)(0xB8000);
      uint16* TestPtr = (uint16*)(0xB8000 + (160*21));
      uint16* Test2Ptr = (uint16*)(0xB8000 + (160*22));

      for (auto Clear = 0; Clear < (80*25); Clear++) {
        Terminal[Clear] = 0;
      }

      int Index = 0;
      while (Test[Index] != '\0') {
        TestPtr[Index] = 0x0F00 + Test[Index];
        Index++;
      }

      Index = 0;
      while (Test2[Index] != '\0') {
        Test2Ptr[Index] = 0x3F00 + Test2[Index];
        Index++;
      }

      return entrypointSuccess;

    } else {

      for (auto y = 0; y < InfoTable->Display.Graphics.LimitY; y++) {

        for (auto x = 0; x < (InfoTable->Display.Graphics.LimitX * InfoTable->Display.Graphics.Bits.PerPixel / 4); x++) {

          *(uint8*)(InfoTable->Display.Graphics.Framebuffer.Address + (InfoTable->Display.Graphics.Pitch * y) + x) = (y * 255 / InfoTable->Display.Graphics.LimitY);

        }

      }

      for (auto a = 0; a < 100000000; a++) {
        __asm__ __volatile__ ("nop");
      }

    }

    return entrypointSuccess;

  }


  // ...

  for(;;);
  return 0; // (0:0)

}
