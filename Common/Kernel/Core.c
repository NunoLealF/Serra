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

      auto Buffer = ((uintptr)GraphicsData.Framebuffer + (GraphicsData.Pitch * Y));
      auto Size = (GraphicsData.LimitX * GraphicsData.Bpp);

      // (Use memset to display a pretty gradient)

      Memset((void*)Buffer, (uint8)(Y * 256 / GraphicsData.LimitY), Size);

    }

  }

  // (Test out Message())

  Message(Kernel, "Hi, welcome to kernel-mode Serra! <3\n\r");

  Message(Info, "InfoTable->Version = %d | InfoTable->Size = %d bytes", InfoTable->Version, InfoTable->Size);

  if (InfoTable->Disk.AccessMethod == UnknownMethod) {

    Message(Info, "InfoTable::Disk.AccessMethod = 0 (`UnknownMethod`)");

  } else if (InfoTable->Disk.AccessMethod == EfiFsMethod) {

    Message(Info, "InfoTable::Disk.AccessMethod = 1 (`EfiFsMethod`)");

    Message(Info, "InfoTable::Disk::EfiFs @ (FileInfo = %xh, HandleList = %xh, Handle = %xh, Protocol = %xh)",
                   InfoTable->Disk.EfiFs.FileInfo.Address, InfoTable->Disk.EfiFs.HandleList.Address,
                   InfoTable->Disk.EfiFs.Handle.Address, InfoTable->Disk.EfiFs.Protocol.Address);

  } else if (InfoTable->Disk.AccessMethod == Int13Method) {

    Message(Info, "InfoTable::Disk.AccessMethod = 2 (`Int13Method`)");

    Message(Info, "InfoTable::Disk::Int13.DriveNumber = %xh", InfoTable->Disk.Int13.DriveNumber);
    Message(Info, "InfoTable::Disk::Int13::Edd @ (IsEnabled = `%s`, Table = %xh)",
                  (InfoTable->Disk.Int13.Edd.IsEnabled ? "true" : "false"),
                   InfoTable->Disk.Int13.Edd.Table.Address);

  }

  if (InfoTable->Display.Type == VgaDisplay) {
    Message(Info, "InfoTable::Display.Type = 1 (`VgaDisplay`)");
  } else if (InfoTable->Display.Type == VbeDisplay) {
    Message(Info, "InfoTable::Display.Type = 2 (`VbeDisplay`)");
  } else if (InfoTable->Display.Type == EfiTextDisplay) {
    Message(Info, "InfoTable::Display.Type = 3 (`EfiTextDisplay`)");
  } else if (InfoTable->Display.Type == GopDisplay) {
    Message(Info, "InfoTable::Display.Type = 4 (`GopDisplay`)");
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

    if (InfoTable->Display.Text.Format == AsciiFormat) {
      Message(Info, "InfoTable::Display::Text.Format = 1 (`AsciiFormat`)");
    } else if (InfoTable->Display.Text.Format == Utf16Format) {
      Message(Info, "InfoTable::Display::Text.Format = 2 (`Utf16Format`)");
    }

    Message(Info, "InfoTable::Display::Text @ (PosX = %d, PosY = %d)",
                   InfoTable->Display.Text.PosX, InfoTable->Display.Text.PosY);

    Message(Info, "InfoTable::Display::Text @ (LimitX = %d, LimitY = %d)",
                   InfoTable->Display.Text.LimitX, InfoTable->Display.Text.LimitY);

  }


  if (InfoTable->Firmware.Type == UnknownFirmware) {

    Message(Info, "InfoTable::Firmware.Type = 0 (`UnknownFirmware`)");

  } else if (InfoTable->Firmware.Type == BiosFirmware) {

    Message(Info, "InfoTable::Firmware.Type = 1 (`BiosFirmware`)");

    if (InfoTable->Firmware.Bios.A20 == EnabledByUnknown) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 0 (`EnabledByUnknown`)");
    } else if (InfoTable->Firmware.Bios.A20 == EnabledByDefault) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 1 (`EnabledByDefault`)");
    } else if (InfoTable->Firmware.Bios.A20 == EnabledByKeyboard) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 2 (`EnabledByKeyboard`)");
    } else if (InfoTable->Firmware.Bios.A20 == EnabledByFast) {
      Message(Info, "InfoTable::Firmware::Bios.A20 = 3 (`EnabledByFast`)");
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

  } else if (InfoTable->Firmware.Type == EfiFirmware) {

    Message(Info, "InfoTable::Firmware.Type = 2 (`EfiFirmware`)");

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

  if (InfoTable->Image.Type == ElfImageType) {
    Message(Info, "InfoTable::Image.Type = 1 (`ElfImageType`)");
  }

  Message(Info, "InfoTable::Image::Executable @ (Entrypoint = %xh, Header = %xh)",
                 InfoTable->Image.Executable.Entrypoint.Address,
                 InfoTable->Image.Executable.Header.Address);

  Message(Info, "InfoTable::Memory @ (NumEntries = %d, List = %xh)",
                 InfoTable->Memory.NumEntries, InfoTable->Memory.List.Address);

  if (InfoTable->System.Architecture == UnknownArchitecture) {
    Message(Info, "InfoTable::System.Architecture = 0 (`UnknownArchitecture`)");
  } else if (InfoTable->System.Architecture == x64Architecture) {
    Message(Info, "InfoTable::System.Architecture = 1 (`x64Architecture`)");
  }

  Message(Info, "InfoTable::System::Acpi @ (IsSupported = `%s`, Table = %xh)",
                (InfoTable->System.Acpi.IsSupported == true ? "true" : "false"),
                 InfoTable->System.Acpi.Table.Address);

  if (InfoTable->System.Architecture == x64Architecture) {

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



  // (Try this out)

  bool Thing = InitializeAllocationSubsystem(InfoTable->Memory.List.Pointer, InfoTable->Memory.NumEntries);

  if (Thing == false) {

    Message(Error, "InitializeAllocationSubsystem() failed for some reason");

  } else {

    Message(Ok, "InitializeAllocationSubsystem() worked");

    const uintptr Size = 0x8800;
    [[maybe_unused]] void* Test1p = Malloc(&Size);

    const uintptr Size2 = 0x8700;
    [[maybe_unused]] void* Test2p = Malloc(&Size2);

    [[maybe_unused]] bool Test1f = Free(Test1p, &Size);
    [[maybe_unused]] bool Test2f = Free(Test2p, &Size2);

    for (uint16 Limit = 0; Limit < 64; Limit++) {

      uint64 Num = 0;

      while (Nodes[Limit] != NULL) {

        Message(Info, "Found a %xh-sized block (at %xh) that represents (%xh, %xh) | (prevS=%xh)",
                      (1ULL << Limit), ((uintptr)Nodes[Limit]),
                      (uintptr)Nodes[Limit]->Pointer, ((uintptr)Nodes[Limit]->Pointer + (1ULL << Limit)),
                      (uintptr)Nodes[Limit]->Size.Previous);

        Nodes[Limit] = Nodes[Limit]->Size.Previous;
        Num++;

      }

      if (Num != 0) {
        Message(Info, "Found %d nodes at logarithm level [%d]", Num, (uint64)Limit);
      }

    }

  }



  // (Depending on the system type, either wait for a keypress or just
  // stall the system for a while)

  if ((InfoTable->Firmware.Type == EfiFirmware) && (InfoTable->Firmware.Efi.SupportsConIn == true)) {

    // (Wait for a keypress)

    efiInputKey PhantomKey;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

  } else {

    // (Do a lot of NOPs to stall the system)

    for (uint64 Count = 0; Count < 5000000000; Count++) {
      __asm__ __volatile__ ("nop");
    }

  }

  // (Return.)

  return;

}
