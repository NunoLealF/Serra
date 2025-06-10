// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Libraries/Stdint.h"
#include "Core.h"

// TODO - Write documentation

void KernelCore(commonInfoTable* InfoTable) {

  // (Depending on the display type, show a graphical demo)

  if (GraphicsData.IsSupported == true) {

    for (auto Y = 0; Y < GraphicsData.LimitY; Y++) {

      // (Get necessary addresses and values)

      auto Buffer = ((uintptr)GraphicsData.Buffer + (GraphicsData.Pitch * Y));
      auto Size = (GraphicsData.LimitX * GraphicsData.Bpp);

      // (Use memset to display a pretty gradient - we first copy to
      // the double buffer though)

      Memset((void*)Buffer, (uint8)(Y * 256 / GraphicsData.LimitY), Size);

    }

    // (Copy the entire double buffer to the framebuffer)

    auto FbSize = (GraphicsData.Pitch * GraphicsData.LimitY);
    Memcpy(GraphicsData.Framebuffer, GraphicsData.Buffer, FbSize);

  }

  // (Test out Message())

  Message(Kernel, "Hi, welcome to kernel-mode Serra! <3\n\r");

  Message(Info, "InfoTable->Version = %d | InfoTable->Size = %d bytes", InfoTable->Version, InfoTable->Size);

  if (InfoTable->Disk.Method == DiskMethod_Unknown) {

    Message(Info, "InfoTable::Disk.Method = 0 (`DiskMethod_Unknown`)");

  } else if (InfoTable->Disk.Method == DiskMethod_Efi) {

    Message(Info, "InfoTable::Disk.Method = 1 (`DiskMethod_Efi`)");

    Message(Info, "InfoTable::Disk::Efi @ (Handle = %xh, Protocol = %xh, FileInfo => %xh)",
                   InfoTable->Disk.Efi.Handle.Address, InfoTable->Disk.Efi.Protocol.Address,
                   InfoTable->Disk.Efi.FileInfo.Address);

  } else if (InfoTable->Disk.Method == DiskMethod_Int13) {

    Message(Info, "InfoTable::Disk.Method = 2 (`DiskMethod_Int13`)");

    Message(Info, "InfoTable::Disk::Int13.DriveNumber = %xh", InfoTable->Disk.Int13.DriveNumber);
    Message(Info, "InfoTable::Disk::Int13::Edd @ (IsSupported = `%s`, Table = %xh, BytesPerSector = %d, NumSectors = %d)",
                  (InfoTable->Disk.Int13.Edd.IsSupported ? "true" : "false"),
                   InfoTable->Disk.Int13.Edd.Table.Address, InfoTable->Disk.Int13.Edd.BytesPerSector,
                   InfoTable->Disk.Int13.Edd.NumSectors);

  }

  if (InfoTable->Display.Type == DisplayType_Vga) {
    Message(Info, "InfoTable::Display.Type = 1 (`DisplayType_Vga`)");
  } else if (InfoTable->Display.Type == DisplayType_Vbe) {
    Message(Info, "InfoTable::Display.Type = 2 (`DisplayType_Vbe`)");
  } else if (InfoTable->Display.Type == DisplayType_EfiText) {
    Message(Info, "InfoTable::Display.Type = 3 (`DisplayType_EfiText`)");
  } else if (InfoTable->Display.Type == DisplayType_Gop) {
    Message(Info, "InfoTable::Display.Type = 4 (`DisplayType_Gop`)");
  }

  if (GraphicsData.IsSupported == true) {

    Message(Info, "InfoTable::Display::Edid.IsSupported = `%s`",
                  (InfoTable->Display.Edid.IsSupported ? "true" : "false"));

    Message(Info, "InfoTable::Display::Edid.PreferredResolution = [%d, %d]",
                   InfoTable->Display.Edid.PreferredResolution[0],
                   InfoTable->Display.Edid.PreferredResolution[1]);

    Message(Info, "InfoTable::Display::Edid.Table = %xh", InfoTable->Display.Edid.Table.Address);

    Message(Info, "InfoTable::Display::Graphics @ (Framebuffer = %xh, Pitch = %d)",
                   InfoTable->Display.Graphics.Framebuffer.Address,
                   InfoTable->Display.Graphics.Pitch);

    Message(Info, "InfoTable::Display::Graphics @ (LimitX = %d, LimitY = %d)",
                   InfoTable->Display.Graphics.LimitX, InfoTable->Display.Graphics.LimitY);

    Message(Info, "InfoTable::Display::Graphics::Bits.PerPixel = %d", InfoTable->Display.Graphics.Bits.PerPixel);
    Message(Info, "InfoTable::Display::Graphics::Bits @ (RedMask = %xh, GreenMask = %xh, BlueMask = %xh)",
                   InfoTable->Display.Graphics.Bits.RedMask, InfoTable->Display.Graphics.Bits.GreenMask,
                   InfoTable->Display.Graphics.Bits.BlueMask);


  } else if (ConsoleData.IsSupported == true) {

    if (InfoTable->Display.Text.Format == TextFormat_Ascii) {
      Message(Info, "InfoTable::Display::Text.Format = 1 (`TextFormat_Ascii`)");
    } else if (InfoTable->Display.Text.Format == TextFormat_Utf16) {
      Message(Info, "InfoTable::Display::Text.Format = 2 (`TextFormat_Utf16`)");
    }

    Message(Info, "InfoTable::Display::Text @ (PosX = %d, PosY = %d)",
                   InfoTable->Display.Text.PosX, InfoTable->Display.Text.PosY);

    Message(Info, "InfoTable::Display::Text @ (LimitX = %d, LimitY = %d)",
                   InfoTable->Display.Text.LimitX, InfoTable->Display.Text.LimitY);

  }


  if (InfoTable->Firmware.Type == FirmwareType_Unknown) {

    Message(Info, "InfoTable::Firmware.Type = 0 (`FirmwareType_Unknown`)");

  } else if (InfoTable->Firmware.Type == FirmwareType_Bios) {

    Message(Info, "InfoTable::Firmware.Type = 1 (`FirmwareType_Bios`)");

    if (InfoTable->Firmware.Bios.A20 == A20_Unknown) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 0 (`A20_Unknown`)");
    } else if (InfoTable->Firmware.Bios.A20 == A20_Default) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 1 (`A20_Default`)");
    } else if (InfoTable->Firmware.Bios.A20 == A20_Keyboard) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 2 (`A20_Keyboard`)");
    } else if (InfoTable->Firmware.Bios.A20 == A20_Fast) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 3 (`A20_Fast`)");
    }

    Message(Info, "InfoTable::Firmware::Bios::Mmap @ (NumEntries = %d, EntrySize = %d)",
                   InfoTable->Firmware.Bios.Mmap.NumEntries, InfoTable->Firmware.Bios.Mmap.EntrySize);

    Message(Info, "InfoTable::Firmware::Bios::Mmap.List = %xh", InfoTable->Firmware.Bios.Mmap.List.Address);

    Message(Info, "InfoTable::Firmware::Bios::Pat @ (IsSupported = `%s`, Value = %xh)",
                  (InfoTable->Firmware.Bios.Pat.IsSupported == true ? "true" : "false"),
                   InfoTable->Firmware.Bios.Pat.Value);

    Message(Info, "InfoTable::Firmware::Bios::PciBios @ (IsSupported = `%s`, Table = %xh)",
                  (InfoTable->Firmware.Bios.PciBios.IsSupported == true ? "true" : "false"),
                   InfoTable->Firmware.Bios.PciBios.Table.Address);

    Message(Info, "InfoTable::Firmware::Bios::Vbe @ (IsSupported = `%s`, ModeNumber = %d)",
                  (InfoTable->Firmware.Bios.Vbe.IsSupported == true ? "true" : "false"),
                   InfoTable->Firmware.Bios.Vbe.ModeNumber);

    Message(Info, "InfoTable::Firmware::Bios::Vbe @ (InfoBlock = %xh, ModeInfo = %xh)",
                   InfoTable->Firmware.Bios.Vbe.InfoBlock.Address,
                   InfoTable->Firmware.Bios.Vbe.ModeInfo.Address);

  } else if (InfoTable->Firmware.Type == FirmwareType_Efi) {

    Message(Info, "InfoTable::Firmware.Type = 2 (`FirmwareType_Efi`)");

    Message(Info, "InfoTable::Firmware::Efi @ (ImageHandle = %xh, SystemTable = %xh)",
                   InfoTable->Firmware.Efi.ImageHandle.Address,
                   InfoTable->Firmware.Efi.SystemTable.Address);

    Message(Info, "InfoTable::Firmware::Efi @ (SupportsConIn = `%s`, SupportsConOut = `%s`)",
                   (InfoTable->Firmware.Efi.SupportsConIn == true ? "true" : "false"),
                   (InfoTable->Firmware.Efi.SupportsConOut == true ? "true" : "false"));

    Message(Info, "InfoTable::Firmware::Efi::Gop @ (IsSupported = `%s`, Protocol = %xh)",
                  (InfoTable->Firmware.Efi.Gop.IsSupported == true ? "true" : "false"),
                   InfoTable->Firmware.Efi.Gop.Protocol.Address);

    Message(Info, "InfoTable::Firmware::Efi::Mmap @ (NumEntries = %d, EntrySize = %d)",
                   InfoTable->Firmware.Efi.Mmap.NumEntries, InfoTable->Firmware.Efi.Mmap.EntrySize);

    Message(Info, "InfoTable::Firmware::Efi::Mmap.List = %xh", InfoTable->Firmware.Efi.Mmap.List.Address);

  }

  Message(Info, "InfoTable::Image @ (StackTop = %xh, StackSize = %d)",
                 InfoTable->Image.StackTop.Address, InfoTable->Image.StackSize);

  if (InfoTable->Image.Type == ImageType_Elf) {
    Message(Info, "InfoTable::Image.Type = 1 (`ImageType_Elf`)");
  }

  Message(Info, "InfoTable::Image::Executable @ (Entrypoint = %xh, Header = %xh)",
                 InfoTable->Image.Executable.Entrypoint.Address,
                 InfoTable->Image.Executable.Header.Address);

  Message(Info, "InfoTable::Memory @ (NumEntries = %d, List = %xh)",
                 InfoTable->Memory.NumEntries, InfoTable->Memory.List.Address);

  if (InfoTable->System.Architecture == SystemArchitecture_Unknown) {
    Message(Info, "InfoTable::System.Architecture = 0 (`SystemArchitecture_Unknown`)");
  } else if (InfoTable->System.Architecture == SystemArchitecture_x64) {
    Message(Info, "InfoTable::System.Architecture = 1 (`SystemArchitecture_x64`)");
  }

  Message(Info, "InfoTable::System::Acpi @ (IsSupported = `%s`, Table = %xh)",
                (InfoTable->System.Acpi.IsSupported == true ? "true" : "false"),
                 InfoTable->System.Acpi.Table.Address);

  if (InfoTable->System.Architecture == SystemArchitecture_x64) {

    Message(Info, "InfoTable::System::Cpu::x64 @ (Cr0 = %xh, Cr3 = %xh)",
                   InfoTable->System.Cpu.x64.Cr0, InfoTable->System.Cpu.x64.Cr3);

    Message(Info, "InfoTable::System::Cpu::x64 @ (Cr4 = %xh, Efer = %xh)",
                   InfoTable->System.Cpu.x64.Cr4, InfoTable->System.Cpu.x64.Efer);

  }

  Message(Info, "InfoTable::System::Smbios @ (IsSupported = `%s`, Table = %xh)",
                (InfoTable->System.Smbios.IsSupported == true ? "true" : "false"),
                 InfoTable->System.Smbios.Table.Address);

  Message(Info, "InfoTable->Checksum = %xh", InfoTable->Checksum);



  // (Display usable memory map, as a test)

  Putchar('\n', false, 0);
  usableMmapEntry* UsableMmap = InfoTable->Memory.List.Pointer;

  for (auto Index = 0; Index < InfoTable->Memory.NumEntries; Index++) {

    Message(Kernel, "Entry [%d] goes from %xh to %xh (%d KiB)",
                     Index, UsableMmap[Index].Base,
                     (UsableMmap[Index].Base + UsableMmap[Index].Limit),
                     (UsableMmap[Index].Limit / 1024));

  }

  // (Show memory allocation nodes and such)

  for (uint16 Limit = 0; Limit < 64; Limit++) {

    uint64 Num = 0;
    allocationNode* Node = MmSubsystemData.Nodes[Limit];

    while (Node != NULL) {

      Message(Info, "Found a %xh-sized block (at %xh) that represents (%xh, %xh) | (prevS=%xh)",
                    (1ULL << Limit), ((uintptr)Node),
                    (uintptr)Node->Pointer, ((uintptr)Node->Pointer + (1ULL << Limit)),
                    (uintptr)Node->Size.Previous);

      Node = Node->Size.Previous;
      Num++;

    }

    if (Num != 0) {
      Message(Info, "Found %d nodes at logarithm level [%d]", Num, (uint64)Limit);
    }

  }

  // (Try to read the MBR from the disk - or, at least, the bootsector,
  // in the case of `unpart` / non-partitioned builds)

  Putchar('\n', false, 0x0F);

  if ((InfoTable->Firmware.Type == FirmwareType_Bios) || (InfoTable->Firmware.Type == FirmwareType_Efi)) {

    const uintptr Size = VolumeList[0].BytesPerSector * 2;
    void* Area = Allocate(&Size);

    if (Area != NULL) {

      // (Define variables)

      auto Lba = 0;

      auto DriveNumber = VolumeList[0].Drive;
      auto NumSectors = Size / VolumeList[0].BytesPerSector;

      // (Show that we did allocate a buffer)

      Message(Ok, "Allocated a %d-byte buffer at %xh.", (uint64)Size, (uint64)Area);

      // (Thing)

      Message(Ok, "Attempting to read %d sectors (from LBA %d of drive %xh) to buffer at %xh.",
                   (uint64)NumSectors, (uint64)Lba, (uint64)DriveNumber, (uint64)Area);

      bool Status;

      if (DiskInfo.BootMethod == BootMethod_Int13) {
        Status = ReadSectors_Bios(Area, Lba, NumSectors, DriveNumber);
      } else {
        Status = ReadSectors_Efi(Area, Lba, NumSectors, DriveNumber);
      }

      // (Depending on the status message...)

      if (Status == true) {

        Message(Ok, "Successfully loaded sectors - showing first %d bytes of the boot disk.", (uint64)Size);
        uint8* Buffer = (uint8*)Area;

        auto ActualSize = NumSectors * VolumeList[0].BytesPerSector;

        Printf("[", false, 0x07); Printf("0h", false, 0x0F); Printf("] ", false, 0x07);

        for (uint64 Index = 0; Index < ActualSize; Index++) {

          if (Buffer[Index] < 0x10) {
            Putchar('0', false, 0x07);
          }

          Printf("%x ", false, 0x07, (uint64)Buffer[Index]);

          if ((Index+1) % 16 == 0) {
            Print("\n\r[", false, 0x07);
            Printf("%xh", false, 0x0F, ((Index+1) / 16)*16);
            Print("] ", false, 0x07);
          }

        }

        Printf("If the read was successful, that should be the data from LBA %d to %d. \n\r",
                false, 0x07, (uint64)Lba, (uint64)Lba+NumSectors);

      } else {

        Message(Warning, "Failed to load sectors :(");

      }

      [[maybe_unused]] bool Thing = Free(Area, &Size);

    } else {

      Message(Error, "Failed to allocate space for buffer :(");

    }

  }

  // (Show a list of disks, hopefully?)

  for (auto Index = 0; Index < NumVolumes; Index++) {

    Message(Kernel, "Found VolumeList[%d] => method(%xh)drive(%xh)partition(%xh)",
                    (uint64)Index, (uint64)VolumeList[Index].Method,
                    (uint64)VolumeList[Index].Drive,
                    (uint64)VolumeList[Index].Partition);

    Message(Info, "Type => %xh; MediaId => %xh; Offset => %xh",
                  (uint64)VolumeList[Index].Type, (uint64)VolumeList[Index].MediaId,
                  (uint64)VolumeList[Index].Offset);

    Message(Info, "Alignment => (1 << %d):(%xh); BytesPerSector => %d; NumSectors => %d",
                  (uint64)VolumeList[Index].Alignment, (1ULL << VolumeList[Index].Alignment),
                  (uint64)VolumeList[Index].BytesPerSector, VolumeList[Index].NumSectors);

    // (Try to read from it!)

    char Bootsector[512];
    bool Status = ReadDisk((void*)Bootsector, 1, 512, (uint16)Index);

    if (Status == true) {

      Message(Ok, "Successfully read the bootsector from volume %d (last four bytes are %xh:%xh:%xh:%xh)",
                  Index, Bootsector[508], Bootsector[509],
                  Bootsector[510], Bootsector[511]);

    } else {

      Message(Error, "Failed to read the bootsector from volume %d :(", Index);

    }

  }

  // (Depending on the system type, either wait for a keypress or just
  // stall the system for a while)

  if ((InfoTable->Firmware.Type == FirmwareType_Efi) && (InfoTable->Firmware.Efi.SupportsConIn == true)) {

    // (Wait for a keypress)

    efiInputKey PhantomKey;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

  } else {

    // (Do a lot of NOPs to stall the system)

    for (uint64 Count = 0; Count < 20000000000; Count++) {
      __asm__ __volatile__ ("nop");
    }

  }

  // (Return.)

  return;

}
