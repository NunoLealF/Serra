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
    return entrypoint_TableIsNull;
  }

  // (Check if the table header contains valid values)

  if (InfoTable->Signature != commonInfoTableSignature) {
    return entrypoint_InvalidSignature;
  } else if (InfoTable->Version != commonInfoTableVersion) {
    return entrypoint_InvalidVersion;
  } else if (InfoTable->Size != sizeof(commonInfoTable)) {
    return entrypoint_InvalidSize;
  }

  // (Calculate our own table checksum, and compare it against the table's)

  uint16 Checksum = 0;
  uint16 ChecksumSize = (InfoTable->Size - sizeof(InfoTable->Checksum));
  uint8* RawData = (uint8*)InfoTable;

  for (auto Offset = 0; Offset < ChecksumSize; Offset++) {
    Checksum += RawData[Offset];
  }

  if (InfoTable->Checksum != Checksum) {
    return entrypoint_InvalidChecksum;
  }

  // (Check if the system architecture is valid)

  if (InfoTable->System.Architecture == UnknownArchitecture) {

    return entrypoint_System_UnknownArchitecture;

  } else if (InfoTable->System.Architecture == x64Architecture) {

    // (Check whether CR0 and CR3 both aren't zero)

    if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return entrypoint_System_InvalidCpuData;
    } else if (InfoTable->System.Cpu.x64.Cr0 == 0) {
      return entrypoint_System_InvalidCpuData;
    }

  } else {

    return entrypoint_System_UnsupportedArchitecture;

  }

  // (Check whether ACPI and SMBIOS data is valid, if provided)

  if (InfoTable->System.Acpi.IsSupported == true) {

    if (InfoTable->System.Acpi.Table.Pointer == NULL) {
      return entrypoint_System_InvalidAcpiData;
    }

  }

  if (InfoTable->System.Smbios.IsSupported == true) {

    if (InfoTable->System.Smbios.Table.Pointer == NULL) {
      return entrypoint_System_InvalidSmbiosData;
    }

  }

  // (Check if the firmware data is valid)

  if (InfoTable->Firmware.Type == UnknownFirmware) {

    return entrypoint_Firmware_UnknownType;

  } else if (InfoTable->Firmware.Type == BiosFirmware) {

    // (BIOS is only supported on x86 systems)

    if (InfoTable->System.Architecture != x64Architecture) {
      return entrypoint_Firmware_UnsupportedType;
    }

    // (Sanity-check the A20 value given)

    if (InfoTable->Firmware.Bios.A20 >= EnabledByMax) {
      return entrypoint_Firmware_InvalidBiosData;
    }

    // (Check whether the BIOS (E820) memory map is valid)

    #define biosMmapEntrySize 24 // (TODO - there should be something like a biosMmapEntry{} definition)

    if (InfoTable->Firmware.Bios.Mmap.NumEntries == 0) {
      return entrypoint_Firmware_InvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.EntrySize < biosMmapEntrySize) {
      return entrypoint_Firmware_InvalidMmapData;
    } else if (InfoTable->Firmware.Bios.Mmap.List.Pointer == NULL) {
      return entrypoint_Firmware_InvalidMmapData;
    }

    // (Check whether the PAT value given is valid, if supported)

    if (InfoTable->Firmware.Bios.Pat.IsSupported == true) {
      if (InfoTable->Firmware.Bios.Pat.Value == 0) {
        return entrypoint_Firmware_InvalidBiosData;
      }
    }

    // (Check whether the PCI BIOS table given is valid, if supported)

    if (InfoTable->Firmware.Bios.PciBios.IsSupported == true) {
      if (InfoTable->Firmware.Bios.PciBios.Table.Pointer == NULL) {
        return entrypoint_Firmware_InvalidBiosData;
      }
    }

    // (Check whether the VBE information given is valid, if supported)

    if (InfoTable->Firmware.Bios.Vbe.IsSupported == true) {

      if (InfoTable->Firmware.Bios.Vbe.InfoBlock.Pointer == NULL) {
        return entrypoint_Firmware_InvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeInfo.Pointer == NULL) {
        return entrypoint_Firmware_InvalidBiosData;
      } else if (InfoTable->Firmware.Bios.Vbe.ModeNumber == 0) {
        return entrypoint_Firmware_InvalidBiosData;
      }

    }

  } else if (InfoTable->Firmware.Type == EfiFirmware) {

    // (Check whether the EFI image handle and system pointer were
    // both correctly passed on)

    if (InfoTable->Firmware.Efi.ImageHandle.Pointer == NULL) {
      return entrypoint_Firmware_InvalidEfiData;
    }

    if (InfoTable->Firmware.Efi.SystemTable.Pointer == NULL) {
      return entrypoint_Firmware_InvalidEfiData;
    }

    // (Check whether the EFI (gBS->GetMemoryMap()) memory map is valid)

    #define efiMmapEntrySize sizeof(efiMemoryDescriptor)

    if (InfoTable->Firmware.Efi.Mmap.NumEntries == 0) {
      return entrypoint_Firmware_InvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.EntrySize < efiMmapEntrySize) {
      return entrypoint_Firmware_InvalidMmapData;
    } else if (InfoTable->Firmware.Efi.Mmap.List.Pointer == NULL) {
      return entrypoint_Firmware_InvalidMmapData;
    }

    // (Check whether the GOP protocol data is valid, if supported)

    if (InfoTable->Firmware.Efi.Gop.IsSupported == true) {
      if (InfoTable->Firmware.Efi.Gop.Protocol.Pointer == NULL) {
        return entrypoint_Firmware_InvalidEfiData;
      }
    }

  } else {

    return entrypoint_Firmware_UnsupportedType;

  }

  // (Check whether the stack data is valid)

  if ((InfoTable->Image.StackTop.Address % 4096) != 0) {
    return entrypoint_Image_InvalidStack;
  } else if ((InfoTable->Image.StackSize % 4096) != 0) {
    return entrypoint_Image_InvalidStack;
  }

  if (InfoTable->Image.StackTop.Pointer == NULL) {
    return entrypoint_Image_InvalidStack;
  } else if (InfoTable->Image.StackSize == 0) {
    return entrypoint_Image_InvalidStack;
  }

  // (Check whether the kernel image data is valid)

  if (InfoTable->Image.Type == UnknownImageType) {
    return entrypoint_Image_UnknownType;
  } else if (InfoTable->Image.Type >= MaxImageType) {
    return entrypoint_Image_UnsupportedType;
  }

  if (InfoTable->Image.Executable.Entrypoint.Pointer == NULL) {
    return entrypoint_Image_InvalidData;
  } else if (InfoTable->Image.Executable.Header.Pointer == NULL) {
    return entrypoint_Image_InvalidData;
  }

  // (Check whether the 'usable' / kernel memory map is valid)

  if (InfoTable->Memory.NumEntries == 0) {
    return entrypoint_Memory_MapHasNoEntries;
  } else if (InfoTable->Memory.List.Pointer == NULL) {
    return entrypoint_Memory_ListIsNull;
  }

  // (Check whether the display data is valid)

  if (InfoTable->Display.Type >= MaxDisplay) {
    return entrypoint_Display_UnsupportedType;
  }

  if (InfoTable->Display.Edid.IsSupported == true) {

    // (Check whether the preferred resolution makes any sense)

    if (InfoTable->Display.Edid.PreferredResolution[0] == 0) {
      return entrypoint_Display_InvalidEdidData;
    } else if (InfoTable->Display.Edid.PreferredResolution[1] == 0) {
      return entrypoint_Display_InvalidEdidData;
    }

    // (Check whether the EDID table is valid or not)

    if (InfoTable->Display.Edid.Table.Pointer == NULL) {
      return entrypoint_Display_InvalidEdidData;
    }

  }

  if ((InfoTable->Display.Type == VgaDisplay) || (InfoTable->Display.Type == EfiTextDisplay)) {

    // (Check whether the text format is supported)

    if (InfoTable->Display.Text.Format == UnknownFormat) {
      return entrypoint_Display_InvalidTextData;
    } else if (InfoTable->Display.Text.Format >= MaxFormat) {
      return entrypoint_Display_InvalidTextData;
    }

    // (Check whether the limits seem sane - we need at least 80*25.)

    if (InfoTable->Display.Text.LimitX < 80) {
      return entrypoint_Display_InvalidTextData;
    } else if (InfoTable->Display.Text.LimitY < 25) {
      return entrypoint_Display_InvalidTextData;
    }

  }

  if ((InfoTable->Display.Type == VbeDisplay) || (InfoTable->Display.Type == GopDisplay)) {

    // (Check whether the framebuffer exists or not)

    if (InfoTable->Display.Graphics.Framebuffer.Pointer == NULL) {
      return entrypoint_Display_InvalidGraphicsData;
    }

    // (Check whether the limits seem sane - we need at least 640*480.)

    if (InfoTable->Display.Graphics.LimitX < 640) {
      return entrypoint_Display_InvalidGraphicsData;
    } else if (InfoTable->Display.Graphics.LimitY < 480) {
      return entrypoint_Display_InvalidGraphicsData;
    }

    // (Check whether the number of bits per pixel seems sane (between 16
    // and 64 bpp, and must be a multiple of 8)

    if ((InfoTable->Display.Graphics.Bits.PerPixel % 8) != 0) {
      return entrypoint_Display_InvalidGraphicsData;
    }

    if (InfoTable->Display.Graphics.Bits.PerPixel < 16) {
      return entrypoint_Display_InvalidGraphicsData;
    } else if (InfoTable->Display.Graphics.Bits.PerPixel > 64) {
      return entrypoint_Display_InvalidGraphicsData;
    }

    // (Check whether the pitch (number of bytes per scanline) looks sane)

    uint64 MinimumPitch = (InfoTable->Display.Graphics.LimitX * (InfoTable->Display.Graphics.Bits.PerPixel / 8));

    if (InfoTable->Display.Graphics.Pitch < MinimumPitch) {
      return entrypoint_Display_InvalidGraphicsData;
    }

    // (Check whether any of the bitmasks overlap each other)

    uint64 Red = InfoTable->Display.Graphics.Bits.RedMask;
    uint64 Green = InfoTable->Display.Graphics.Bits.GreenMask;
    uint64 Blue = InfoTable->Display.Graphics.Bits.BlueMask;

    if ((Red & Green) != 0) {
      return entrypoint_Display_InvalidGraphicsData;
    } else if ((Red & Blue) != 0) {
      return entrypoint_Display_InvalidGraphicsData;
    } else if ((Green & Blue) != 0) {
      return entrypoint_Display_InvalidGraphicsData;
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
      return entrypoint_Display_InvalidGraphicsData;
    }

  }

  // (Check whether the disk/filesystem data is valid)

  if (InfoTable->Disk.AccessMethod == UnknownMethod) {
    return entrypoint_Disk_UnknownMethod;
  } else if (InfoTable->Disk.AccessMethod >= MaxMethod) {
    return entrypoint_Disk_UnsupportedMethod;
  }

  if (InfoTable->Disk.AccessMethod == Int13Method) {

    // (This method is only valid on systems with BIOS firmware)

    if (InfoTable->Firmware.Type != BiosFirmware) {
      return entrypoint_Disk_UnsupportedMethod;
    }

    // (Check if the drive number is valid (00-0Fh, or 80-8Fh)

    if ((InfoTable->Disk.Int13.DriveNumber > 0x10) && (InfoTable->Disk.Int13.DriveNumber < 0x80)) {
      return entrypoint_Disk_InvalidData;
    } else if (InfoTable->Disk.Int13.DriveNumber > 0x90) {
      return entrypoint_Disk_InvalidData;
    }

    // (Check if EDD data is valid)

    if (InfoTable->Disk.Int13.Edd.IsEnabled == true) {
      if (InfoTable->Disk.Int13.Edd.Table.Pointer == NULL) {
        return entrypoint_Disk_InvalidData;
      }
    }

  } else if (InfoTable->Disk.AccessMethod == EfiFsMethod) {

    // (This method is only valid on systems with EFI firmware)

    if (InfoTable->Firmware.Type != EfiFirmware) {
      return entrypoint_Disk_UnsupportedMethod;
    }

    // (Check for null pointers)

    if (InfoTable->Disk.EfiFs.FileInfo.Pointer == NULL) {
      return entrypoint_Disk_InvalidData;
    } else if (InfoTable->Disk.EfiFs.HandleList.Pointer == NULL) {
      return entrypoint_Disk_InvalidData;
    } else if (InfoTable->Disk.EfiFs.Handle.Pointer == NULL) {
      return entrypoint_Disk_InvalidData;
    } else if (InfoTable->Disk.EfiFs.Protocol.Pointer == NULL) {
      return entrypoint_Disk_InvalidData;
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
    if (InfoTable->Firmware.Efi.SupportsConIn == false) goto ReturnToEfi;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

    ReturnToEfi:

    __asm__ __volatile__ ("cli");
    return entrypoint_Success;

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

      return entrypoint_Success;

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

    return entrypoint_Success;

  }


  // ...

  for(;;);
  return 0; // (0:0)

}
