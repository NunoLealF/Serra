// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

/* efiSystemTable* gST, efiBootServices* gBS, efiRuntimeServices* gRT

   Definitions: (Efi/Efi.h, Efi/Tables.h)

   These are global pointers to their respective EFI tables; they're
   declared here, initialized in SEfiBootloader(), and can be used
   by *any* function that uses the Efi/Efi.h header.

*/

efiSystemTable* gST;
efiBootServices* gBS;
efiRuntimeServices* gRT;


/* efiStatus efiAbi SEfiBootloader()

   Inputs: efiHandle ImageHandle - The firmware-provided image handle.

           efiSystemTable* SystemTable - A pointer to the firmware-provided
           system table.

   Outputs: ???

   TODO - ...

*/

efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable) {

  // [Prepare global variables]

  KernelInfoTable.Firmware.IsEfi = true;
  KernelInfoTable.Firmware.EfiInfo.Address = (uintptr)(&EfiInfoTable);
  EfiInfoTable.ImageHandle.Ptr = ImageHandle;

  efiStatus AppStatus = EfiSuccess;

  // [Make sure the firmware-provided tables are valid]

  // First and foremost, we need to check to see if the tables our firmware
  // gave us are even valid. We can do this by checking their signature
  // and size, like this:

  // (Check EFI System Table, and update gST)

  if (SystemTable == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Signature != efiSystemTableSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Size < sizeof(efiSystemTable)) {
    return EfiInvalidParameter;
  } else if (SystemTable->Hdr.Revision < calculateRevision(1, 1)) {
    return EfiUnsupported; // We don't support anything below EFI 1.1
  }

  EfiInfoTable.SystemTable.Ptr = SystemTable;
  gST = SystemTable;

  // (Check EFI Boot Services table, and update gBS)

  if (SystemTable->BootServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Signature != efiBootServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Size < sizeof(efiBootServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.BootServices.Ptr = SystemTable->BootServices;
  gBS = gST->BootServices;

  // (Check EFI Runtime Services table, and update gRT)

  if (SystemTable->RuntimeServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Signature != efiRuntimeServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Size < sizeof(efiRuntimeServices)) {
    return EfiInvalidParameter;
  }

  EfiInfoTable.RuntimeServices.Ptr = SystemTable->RuntimeServices;
  gRT = gST->RuntimeServices;



  // [Enable any necessary x64 features]

  // After that, we also want to make sure SSE and other x64 features are
  // enabled, since some firmware doesn't do that by default. Before that
  // though, let's save the current state of the control registers:

  KernelInfoTable.System.Cr0 = ReadFromControlRegister(0);
  KernelInfoTable.System.Cr3 = ReadFromControlRegister(3);
  KernelInfoTable.System.Cr4 = ReadFromControlRegister(4);
  KernelInfoTable.System.Efer = ReadFromMsr(0xC0000080);

  // (Enable the FPU/MMX, by setting the necessary bits in CR0)

  uint32 Cr0 = ReadFromControlRegister(0);

  Cr0 = setBit(Cr0, 1); // Set CR0.MP (1 << 1)
  Cr0 = setBit(Cr0, 4); // Set CR0.ET (1 << 4)
  Cr0 = setBit(Cr0, 5); // Set CR0.NE (1 << 5)

  WriteToControlRegister(0, Cr0);

  // (Enable SSE, by setting the necessary bits in CR4)

  uint32 Cr4 = ReadFromControlRegister(4);

  Cr4 = setBit(Cr4, 9); // Set CR4.OSFXSR (1 << 9)
  Cr4 = setBit(Cr4, 10); // Set CR4.OSXMMEXCPT (1 << 10)

  WriteToControlRegister(4, Cr4);



  // [Check availability of ConIn and ConOut]

  // Next, we also want to do the same thing, but for the input and output
  // protocols (SystemTable->ConIn and SystemTable->ConOut); it's not a
  // *deal breaker* if they aren't supported, we just want to make sure.

  // (Check for ConIn / efiSimpleTextInputProtocol)

  SupportsConIn = true;
  efiInputKey PhantomKey;

  if (gST->ConsoleInHandle == NULL) {
    SupportsConIn = false;
  } else if (gST->ConIn == NULL) {
    SupportsConIn = false;
  } else if (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiUnsupported) {
    SupportsConIn = false;
  } else {
    gST->ConIn->Reset(gST->ConIn, true);
  }

  // (Check for ConOut / efiSimpleTextOutputProtocol)

  SupportsConOut = true;
  char16* PhantomString = u" ";

  if (gST->ConsoleOutHandle == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut->Mode == NULL) {
    SupportsConOut = false;
  } else if (gST->ConOut->OutputString(gST->ConOut, PhantomString) == EfiUnsupported) {
    SupportsConOut = false;
  } else {
    gST->ConOut->Reset(gST->ConOut, true);
  }



  // [Switch to largest text mode, if applicable]

  // Now that we know whether we can use EFI's text mode (by checking
  // SupportsConOut), we want to automatically switch to the best
  // text mode available.

  int32 ConOutMode = 0;
  uint32 ConOutResolution[2] = {0};

  if (SupportsConOut == true) {

    // EFI mode numbers run between 0 and (MaxMode - 1), so, we first
    // need to look for MaxMode

    int32 MaxMode = gST->ConOut->Mode->MaxMode;

    for (int32 Mode = 0; Mode < MaxMode; Mode++) {

      // Query mode information; we want to filter for modes that are
      // at least 80*25 characters, and with the largest overall area.

      uint64 Columns, Rows;

      if (gST->ConOut->QueryMode(gST->ConOut, Mode, &Columns, &Rows) == EfiSuccess) {

        if ((Columns >= 80) && (Rows >= 25)) {

          // If this text mode is larger than 80*25 characters and has
          // a larger overall area, then select it as our best mode.

          if ((Columns * Rows) >= (ConOutResolution[0] * ConOutResolution[1])) {

            ConOutMode = Mode;

            ConOutResolution[0] = Columns;
            ConOutResolution[1] = Rows;

          }

        }

      }

    }

    // Now that we know the best text mode to use, let's switch to it!

    gST->ConOut->SetMode(gST->ConOut, ConOutMode);

  }

  // (Show a few initial messages, now that we've set up the console)

  Message(Boot, u"Successfully initialized the console.");

  Message(Info, u"This system's EFI revision is %d.%d", (uint16)(gST->Hdr.Revision >> 16), (uint16)(gST->Hdr.Revision));
  Message(Info, u"Using text mode %d, with a %d*%d character resolution", ConOutMode, ConOutResolution[0], ConOutResolution[1]);



  // [Check for GOP support]

  // Next, we also want to check for graphics mode support, since not all
  // systems support EFI text mode.

  // Thankfully, this is actually pretty easy; we just need to check for
  // the existence of the Graphics Output Protocol (GOP), like this:

  Print(u"\n\r", 0);
  Message(Boot, u"Checking for UEFI GOP (Graphics Output Protocol) support.");

  efiGraphicsOutputProtocol* GopProtocol;
  efiUuid GopUuid = efiGraphicsOutputProtocol_Uuid;
  bool SupportsGop = false;

  if (gBS->LocateProtocol(&GopUuid, NULL, (void**)(&GopProtocol)) == EfiSuccess) {

    KernelInfoTable.Graphics.Type = Gop;
    SupportsGop = true;

    Message(Ok, u"Successfully located a GOP protocol instance using LocateProtocol().");
    Message(Info, u"Protocol instance is located at %xh", (uintptr)GopProtocol);

  } else {

    // (If we didn't find one, that's fine, as long as text mode is
    // supported; otherwise, return)

    if (SupportsConOut == true) {
      Message(Warning, u"Unable to initialize GOP; keeping EFI text mode.");
    } else {
      AppStatus = EfiUnsupported;
      goto ExitEfiApplication;
    }

  }

  // (Update the graphics type in the kernel info table)

  KernelInfoTable.Graphics.Type = ((SupportsGop == true) ? Gop : EfiText);



  // [Find GOP mode, if applicable]

  // If the firmware *does* support GOP, then the next thing we want
  // to do is try to find the best graphics mode (that fits our
  // criteria, at least).

  uint32 GopMode = 0;

  uint8 GopColorDepth = 0;
  uint32 GopResolution[2] = {0};

  if (SupportsGop == true) {

    Print(u"\n\r", 0);
    Message(Boot, u"Attempting to find the best graphics mode.");

    // Just like with EFI text mode, GOP graphics modes also run between
    // 0 and (MaxMode - 1), so we first need to look for that.

    uint32 MaxMode = GopProtocol->Mode->MaxMode;

    // Now that we know the mode limit, we can manually go through
    // each mode and try to find the best one, if possible.

    for (uint32 Mode = 0; Mode < MaxMode; Mode++) {

      efiGraphicsOutputModeInformation* ModeInfo;
      uint64 Size;

      if (GopProtocol->QueryMode(GopProtocol, Mode, &Size, &ModeInfo) == EfiSuccess) {

        // If the pixel format is PixelBltOnly, then that means it's
        // not a linear framebuffer, so make sure it's not that

        if (ModeInfo->PixelFormat < PixelBltOnly) {

          // (Check and verify color depth - must be at least 16 bpp)

          uint8 ColorDepth = 24;

          if (ModeInfo->PixelFormat == PixelBitMask) {

            // For each bit set in the reserved mask, decrement ModeColorDepth
            // This is essentially just (32 - popcount(ReservedMask))

            ColorDepth = 32;

            for (int Bit = 0; Bit < 32; Bit++) {

              if ((ModeInfo->PixelBitmask.ReservedMask & (1 << Bit)) != 0) {
                ColorDepth--;
              }

            }

          }

          if (ColorDepth < 16) {

            Message(Warning, u"Skipping GOP mode %d (color depth is too low)");
            continue;

          }

          // Now that we know the color depth isn't too low, we check for
          // the following things, in order:

          // (1) Whether this mode's resolution is at least 640 by 480;
          // (2) Whether this mode's color depth is above the current maximum;
          // (3) Whether this mode's width is above or equal to the current maximum;

          if ((ModeInfo->HorizontalResolution >= 640) && (ModeInfo->VerticalResolution >= 480)) {

            if (ColorDepth >= GopColorDepth) {

              // In this case, we actually want to filter modes by their
              // horizontal resolution, not necessarily their area

              if (ModeInfo->HorizontalResolution >= GopResolution[0]) {

                GopMode = Mode;

                GopColorDepth = ColorDepth;
                GopResolution[0] = ModeInfo->HorizontalResolution;
                GopResolution[1] = ModeInfo->VerticalResolution;

              }

            }

          }

        } else {

          Message(Warning, u"Skipping GOP mode %d (unsupported pixel format %d).", Mode, ModeInfo->PixelFormat);
          continue;

        }

      }

    }

    // (Show the mode number, as well as the resolution / color depth)

    Message(Ok, u"Found GOP mode %d, with a %d*%d resolution and %d-bit color depth.",
            (uint64)GopMode, (uint64)GopResolution[0], (uint64)GopResolution[1], (uint64)GopColorDepth);

  }



  // [Obtain the system memory map]

  // After that, we want to move onto memory management - our kernel needs
  // to know about the system's memory map (so it knows where to allocate
  // pages).

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to obtain the system's EFI memory map.");

  // Thankfully, EFI makes this really easy for us; we just need to call
  // GetMemoryMap(). First though, we need to query the size of the
  // memory map, like this:

  // (Declare initial variables)

  volatile void* Mmap = NULL;

  uint64 MmapKey;
  volatile uint64 MmapSize = 0;

  volatile uint64 MmapDescriptorSize = 0;
  volatile uint32 MmapDescriptorVersion = 0;

  // (Call gBS->GetMemoryMap(), with *MmapSize == 0; this will always return
  // EfiBufferTooSmall, and return the correct buffer size in MmapSize)

  AppStatus = gBS->GetMemoryMap(&MmapSize, Mmap, &MmapKey, &MmapDescriptorSize, &MmapDescriptorVersion);

  if (AppStatus != EfiBufferTooSmall) {

    if ((MmapSize != 0) && (MmapDescriptorSize != 0)) {

      Message(Warning, u"Initial call to GetMemoryMap() appears to be badly implemented.");

    } else {

      Message(Fail, u"GetMemoryMap() appears to be working incorrectly.");
      goto ExitEfiApplication;

    }

  }

  Message(Ok, u"Initial call to GetMemoryMap() indicates memory map is %d bytes long.", (uint64)MmapSize);

  // (Allocate space for the memory map, with some margin for error; we also
  // do the same for the usable memory map, which we'll fill out later on.)

  kernelUsableMmap* UsableMmap;

  MmapSize += (MmapDescriptorSize * 4);
  AppStatus = gBS->AllocatePool(EfiLoaderData, MmapSize, (volatile void**)&Mmap);

  if (AppStatus == EfiSuccess) {

    // (Show message)

    Message(Ok, u"Successfully allocated a %d-byte buffer for the memory map.", MmapSize);
    Message(Info, u"Memory map buffer is located at %xh", (uint64)Mmap);

    // (Repeat the process, for the usable memory map buffer.)

    uint64 UsableMmapSize = (MmapSize * sizeof(kernelUsableMmap) / MmapDescriptorSize);
    AppStatus = gBS->AllocatePool(EfiLoaderData, UsableMmapSize, (volatile void**)&UsableMmap);

    if (AppStatus == EfiSuccess) {

      Message(Ok, u"Successfully allocated a %d-byte buffer for the usable memory map.", UsableMmapSize);
      Message(Info, u"Usable memory map buffer is located at %xh", (uint64)UsableMmap);

    } else {

      Message(Fail, u"Failed to allocate space for the usable memory map.");
      goto ExitEfiApplication;

    }

  } else {

    Message(Fail, u"Failed to allocate space for the memory map.");
    goto ExitEfiApplication;

  }

  // (Fill out the memory map, and show all usable entries)

  AppStatus = gBS->GetMemoryMap(&MmapSize, Mmap, &MmapKey, &MmapDescriptorSize, &MmapDescriptorVersion);

  if (AppStatus == EfiSuccess) {

    Message(Ok, u"Successfully obtained the system memory map.");

    for (uint64 EntryNum = 0; EntryNum < (MmapSize / MmapDescriptorSize); EntryNum++) {

      efiMemoryDescriptor* Entry = GetMmapEntry(Mmap, MmapDescriptorSize, EntryNum);

      uint64 EntryStart = Entry->PhysicalStart;
      uint64 EntryEnd = (EntryStart + (Entry->NumberOfPages * 0x1000));

      switch (Entry->Type) {

        case EfiConventionalMemory:
          Message(Info, u"EfiConventionalMemory spans %xh to %xh", EntryStart, EntryEnd);
          break;

        case EfiLoaderCode:
          Message(Info, u"EfiLoaderCode spans %xh to %xh", EntryStart, EntryEnd);
          break;

        case EfiLoaderData:
          Message(Info, u"EfiLoaderData spans %xh to %xh", EntryStart, EntryEnd);
          break;

        default:
          continue;

      }

    }

  } else {

    Message(Fail, u"Failed to obtain the system memory map.");
    goto ExitEfiApplication;

  }



  // [Process the system memory map]
  // TODO: Some sort of explanation.                                                                 TODO TODO TODO

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to process the system memory map.");

  // First, let's sort each entry by their starting address. Since we can't
  // dynamically allocate memory without changing the memory map, we need
  // an algorithm that uses O(1) (no additional) memory, like this:

  uint64 NumMmapEntries = (MmapSize / MmapDescriptorSize);

  if (NumMmapEntries >= 65536) {

    Message(Warning, u"Too many memory map entries - limiting to 65536.");
    NumMmapEntries = 65535;

  }

  for (uint16 Threshold = 1; Threshold < NumMmapEntries; Threshold++) {

    for (uint16 Position = Threshold; Position > 0; Position--) {

      // (Get the actual entries themselves)

      efiMemoryDescriptor* PreviousEntry = GetMmapEntry(Mmap, MmapDescriptorSize, (Position - 1));
      efiMemoryDescriptor* Entry = GetMmapEntry(Mmap, MmapDescriptorSize, Position);

      // (Sort by starting address, and if they're same, make sure the
      // entry with the non-usable type comes first.)

      if (Entry->PhysicalStart < PreviousEntry->PhysicalStart) {

        Memswap((void*)Entry, (void*)PreviousEntry, MmapDescriptorSize);

      } else if (Entry->PhysicalStart == PreviousEntry->PhysicalStart) {

        if (Entry->Type != EfiConventionalMemory) {
          Memswap((void*)Entry, (void*)PreviousEntry, MmapDescriptorSize);
        }

      }

    }

  }

  // Now that we've sorted the memory map, we can move onto the next part:
  // actually building the usable memory map.

  uint16 NumUsableMmapEntries = 0;
  uint64 UsableMemory = 0;

  // (In this case, the usable memory map is simply just a 'simplified'
  // memory map that accounts for overlapping and duplicate entries, and
  // that *only* contains free/usable memory)

  uint16 MmapPositionThreshold = 0;
  uint64 UsableMmapThreshold = 0;

  for (uint16 Position = 0; Position < NumMmapEntries; Position++) {

    // Find the first entry that starts on or at UsableMmapThreshold, of
    // type EfiConventionalMemory.

    efiMemoryDescriptor* Entry = NULL;

    for (uint16 MmapPosition = MmapPositionThreshold; MmapPosition < NumMmapEntries; MmapPosition++) {

      efiMemoryDescriptor* MmapEntry = GetMmapEntry(Mmap, MmapDescriptorSize, MmapPosition);

      if (MmapEntry->Type == EfiConventionalMemory) {

        if (MmapEntry->PhysicalStart >= UsableMmapThreshold) {

          Entry = MmapEntry;

          MmapPositionThreshold = (MmapPosition + 1);
          UsableMmapThreshold = MmapEntry->PhysicalStart;

          break;

        }

      }

    }

    if (Entry == NULL) {
      break;
    }

    // Now that we've found it, let's compare it with the starting position
    // of the next entry, and update the entry size if the two overlap.

    uint64 Start = Entry->PhysicalStart;
    uint64 End = (Start + (Entry->NumberOfPages * 0x1000));

    if (MmapPositionThreshold < NumMmapEntries) {

      efiMemoryDescriptor* NextEntry = GetMmapEntry(Mmap, MmapDescriptorSize, MmapPositionThreshold);

      if (NextEntry->PhysicalStart < End) {
        End = NextEntry->PhysicalStart;
      }

    }

    // Finally, let's add it to the usable memory map.

    UsableMmap[NumUsableMmapEntries].Base = Start;
    UsableMmap[NumUsableMmapEntries].Limit = (End - Start);
    UsableMemory += (End - Start);

    NumUsableMmapEntries++;

  }

  // (Sanity-check the output, and show infomation; *we want to make sure
  // we have at least MinUsableMemory bytes available*.)

  if (NumUsableMmapEntries == 0) {

    Message(Fail, u"Unable to process the system memory map.");

    AppStatus = EfiOutOfResources;
    goto ExitEfiApplication;

  } else if (UsableMemory < MinUsableMemory) {

    Message(Warning, u"Found less usable memory than the recommended amount (%d/%d MiB).",
            (UsableMemory / 1048576), (MinUsableMemory / 1048576));

  } else {

    Message(Ok, u"Found %d usable memory areas, for a total of %d MiB.",
            (uint64)NumUsableMmapEntries, (UsableMemory / 1048576));

  }

  // (Update the kernel info table with the necessary values)

  KernelInfoTable.Memory.NumMmapEntries = NumMmapEntries;
  KernelInfoTable.Memory.MemoryMapEntrySize = MmapDescriptorSize;
  KernelInfoTable.Memory.MemoryMap.Address = (uintptr)Mmap;

  KernelInfoTable.Memory.NumUsableMmapEntries = NumUsableMmapEntries;
  KernelInfoTable.Memory.UsableMemoryMap.Address = (uintptr)UsableMmap;



  // [Find ACPI and SMBIOS tables]
  // TODO: Some sort of explanation.                                                                 TODO TODO TODO

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to find ACPI and SMBIOS tables.");

  // (Prepare variables)

  void* AcpiTable = NULL;
  bool SupportsAcpi = false;
  bool SupportsNewAcpi = false;

  void* SmbiosTable = NULL;
  bool SupportsSmbios = false;
  bool SupportsNewSmbios = false;

  // (Go through each configuration table entry, in the system table)

  for (uint64 Entry = 0; Entry < gST->NumberOfTableEntries; Entry++) {

    efiConfigurationTable* Table = &(gST->ConfigurationTable[Entry]);

    // (Scan for ACPI 1 tables, making sure to not overwrite any existing
    // ACPI 2+ tables if they exist.)

    if (Memcmp(&Table->VendorGuid, &efiAcpiTable_Uuid, sizeof(efiUuid))) {

      if (SupportsNewAcpi == false) {
        AcpiTable = Table->VendorTable;
      }

      SupportsAcpi = true;

      Message(Info, u"Found ACPI 1 table at %xh", (uintptr)Table->VendorTable);
      continue;

    }

    // (Scan for ACPI 2+ tables.)

    if (Memcmp(&Table->VendorGuid, &efiAcpi2Table_Uuid, sizeof(efiUuid))) {

      SupportsNewAcpi = true;
      AcpiTable = Table->VendorTable;

      Message(Info, u"Found ACPI 2+ table at %xh", (uintptr)Table->VendorTable);
      continue;

    }

    // (Scan for SMBIOS 1/2 tables, making sure not to overwrite any existing
    // SMBIOS 3+ tables if they exist.)

    if (Memcmp(&Table->VendorGuid, &efiSmbiosTable_Uuid, sizeof(efiUuid))) {

      if (SupportsNewSmbios == false) {
        SmbiosTable = Table->VendorTable;
      }

      SupportsSmbios = true;

      Message(Info, u"Found SMBIOS 1/2 table at %xh", (uintptr)Table->VendorTable);
      continue;

    }

    // (Scan for SMBIOS 3+ tables.)

    if (Memcmp(&Table->VendorGuid, &efiAcpi2Table_Uuid, sizeof(efiUuid))) {

      SupportsNewSmbios = true;
      SmbiosTable = Table->VendorTable;

      Message(Info, u"Found SMBIOS 3+ table at %xh", (uintptr)Table->VendorTable);
      continue;

    }

  }

  // (Fill out the kernel info table, and show information.)

  if ((SupportsAcpi == true) || (SupportsNewAcpi == true)) {

    KernelInfoTable.System.AcpiSupported = true;
    KernelInfoTable.System.AcpiRsdp.Ptr = AcpiTable;
    Message(Ok, u"ACPI appears to be present (using table at %xh).", AcpiTable);

  } else {

    KernelInfoTable.System.AcpiSupported = false;
    Message(Warning, u"ACPI appears to be unsupported.");

  }

  if ((SupportsSmbios == true) || (SupportsNewSmbios == true)) {

    KernelInfoTable.System.SmbiosSupported = true;
    KernelInfoTable.System.SmbiosTable.Ptr = SmbiosTable;
    Message(Ok, u"SMBIOS appears to be present (using table at %xh).", SmbiosTable);

  } else {

    KernelInfoTable.System.SmbiosSupported = false;
    Message(Warning, u"SMBIOS appears to be unsupported.");

  }







  // TODO (List things to do)

  /*

  Print(u"\n\r", 0);
  Message(-1, u"TODO - Load and allocate space for kernel image");
  Message(Info, u"Preferably, do this before you deal with the memory map");

  Print(u"\n\r", 0);
  Message(-1, u"TODO - Locate any necessary protocols, and try to do any remaining\n\rprep work.");

  */



  // TODO (Show Serra msg thing)

  DebugFlag = true;

  Print(u"\n\r", 0);
  Print(u"Hi, this is EFI-mode Serra! <3 \n\r", 0x0F);
  Printf(u"May %d %x", 0x3F, 6, 0x2025);

  // TODO (Wait until user strikes a key, then return.)

  Print(u"\n\n\r", 0);

  if (SupportsConIn == true) {
    Message(Warning, u"Press any key to return.");
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);
  }

  AppStatus = EfiSuccess;
  goto ExitEfiApplication;

  // TODO (Actually calling the kernel, i guess?)

  // ---------------- Restore state and exit EFI application ----------------

  ExitEfiApplication:

    // If text mode / ConOut is enabled, then restore regular text mode
    // (which is always mode 0)

    if (SupportsConOut == true) {
      gST->ConOut->SetMode(gST->ConOut, 0);
    }

    // Restore the initial values of the CR0 and CR4 registers, since we
    // modified them to set up the FPU and enable SSE.

    WriteToControlRegister(0, KernelInfoTable.System.Cr0); // Restore CR0
    WriteToControlRegister(4, KernelInfoTable.System.Cr4); // Restore CR4

    // Return whatever is in AppStatus

    return AppStatus;

}
