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

  for (uint16 Limit = MmSubsystemData.Limits[0]; Limit < MmSubsystemData.Limits[1]; Limit++) {

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

  // (Initialize the filesystem/partition subsystem - it's still not ready,
  // so we initialize it here rather than in Entry.c to catch error
  // messages and such)

  Print("\n\r", false, 0x0F);
  [[maybe_unused]] bool Thing2 = InitializeFsSubsystem();

  // (NOTE - Sim, isto é a única parte do projeto inteiro em português)

  // (O font renderer que estou a usar só aceita caracteres ASCII ingleses
  // por padrão, e é o mesmo para os modos de texto - é possível mostrar
  // caracteres com acentos, mas não é estandardizado)

  // (Mostrar um demo gráfico)

  if (ConsoleData.Type == GraphicalConsole) {

    // (Fazer print normalmente - isto é só para atualizar `PosX` e `PosY`
    // em `ConsoleData`)

    Print("[Serra] - um novo boot manager.", true, 0x0F);

    // (Deixar o fundo ficar mais e mais azul)

    for (uint64 Multiplier = 0; Multiplier < 0x80; Multiplier++) {

      for (uint64 Delay = 0; Delay < 10000 * (0x80 - Multiplier); Delay++) {
        __asm__ __volatile__ ("nop");
      }

      DrawBitmapFont("[Serra] - um novo boot manager.", &BitmapFontData,
                      0xFFFFFF, (Multiplier * 0x0101), 0,
                      (ConsoleData.PosY * BitmapFontData.Height));

    }

    // (Quando o fundo finalmente ficar azul, mostrar um gradiante
    // da esquerda à direita)

    auto Start = (ConsoleData.PosX * BitmapFontData.Width);
    auto End = GraphicsData.LimitX;

    for (uint64 X = Start; X < End; X++) {

      for (uint64 Delay = 0; Delay < 500 * (End - X); Delay++) {
        __asm__ __volatile__ ("nop");
      }

      uint8 Multiplier = 0x80 - (uint8)(((X - Start) * 0x80) / (End - Start));

      DrawRectangle((Multiplier * 0x0101), X,
                    (ConsoleData.PosY * BitmapFontData.Height), 1, BitmapFontData.Height);

    }

  } else {

    // (Se não for possível mostrar o demo gráfico (por estar em modo de
    // texto), mostrar normalmente com 3Fh (fundo azul + texto branco))

    Print("[Serra] - um novo boot manager.", true, 0x3F);

  }

  // (Mostrar string informativa)

  Print("\n\r", true, 0x0F);
  Print("Nuno Filipe Leal Faria (n. 15962) - TGPSI3 (Escola da APEL) - ", true, 0x0F);

  // Se a console está limitado a menos do que 82 caracteres, não há espaço
  // para mostrar "14 de junho de 2025", só "14 junho 2025":

  if (ConsoleData.LimitX < 82) {
    Print("14 junho 2025 \n\r", true, 0x0F);
  } else {
    Print("14 de junho de 2025 \n\r", true, 0x0F);
  }

  // A font gráfica que estou a usar não suporta caracteres especiais de forma
  // 'normal', mas ainda consegui descubrir que:

  // (Font gráfica / Tamzen) \x1C == 'ã', \x82 == 'é', \x87 == 'ç'
  // (VGA / code page 437) \x83 == 'ã', \x82 == 'é', \x87 == 'ç'
  // (EFI) A maioria dos sistemas não suportam caracteres especiais

  // (Por isso, dependendo do valor de `ConsoleData.Type`, eu acabo por
  // mostrar uma string diferente)

  if (ConsoleData.Type == GraphicalConsole) {
    Printf("Prova de Aptid%co Profissional (T%ccnico de Gest%co e Programa%c%co de Sist. Inf.)\n\r", true, 0x07, 0x1C, 0x82, 0x1C, 0x87, 0x1C); // Tamzen (não parece ser standard)
  } else if (ConsoleData.Type == VgaConsole) {
    Printf("Prova de Aptid%co Profissional (T%ccnico de Gest%co e Programa%c%co de Sist. Inf.)\n\r", true, 0x07, 0x83, 0x82, 0x83, 0x87, 0x83); // VGA (code page 437)
  } else if (ConsoleData.Type == EfiConsole) {
    Printf("Prova de Aptidao Profissional (Tecnico de Gestao e Programacao de Sist. Inf.)\n\r", true, 0x07); // EFI (normalmente não suporta acentos (?))
  }

  // (Depending on the system type, either wait for a keypress or just
  // stall the system for a while)

  if ((InfoTable->Firmware.Type == FirmwareType_Efi) && (InfoTable->Firmware.Efi.SupportsConIn == true)) {

    // (Wait for a keypress)

    Print("\n\r", true, 0x07);
    Message(Warning, "Press any key to return.");

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
