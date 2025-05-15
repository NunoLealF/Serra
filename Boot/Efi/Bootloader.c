// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "Stdint.h"
#include "Bootloader.h"

/* efiStatus efiAbi SEfiBootloader()

   Inputs: efiHandle ImageHandle - The firmware-provided image handle.

           efiSystemTable* SystemTable - A pointer to the firmware-provided
           system table.

   Outputs: ???

   TODO - ...

*/

efiStatus efiAbi SEfiBootloader(efiHandle ImageHandle, efiSystemTable* SystemTable) {

  // [Initialize important variables]

  // Before we do anything else, we'll want to prepare a few important
  // variables and tables; namely, the common information table, the return
  // value of this function, as well as a couple other local variables.

  // (Prepare global variables)

  CommonInfoTable.Signature = commonInfoTableSignature;
  CommonInfoTable.Version = commonInfoTableVersion;
  CommonInfoTable.Size = sizeof(CommonInfoTable);

  CommonInfoTable.Firmware.Type = EfiFirmware;
  CommonInfoTable.Firmware.Efi.ImageHandle.Pointer = ImageHandle;

  CommonInfoTable.Image.DebugFlag = DebugFlag;
  CommonInfoTable.System.Architecture = x64Architecture;

  // (Prepare local variables)

  efiStatus AppStatus = EfiSuccess;

  bool HasAllocatedMemory = false;
  bool HasAllocatedKernel = false;
  bool HasAllocatedStack = false;

  bool HasOpenedFsHandles = false;
  bool HasOpenedGopHandles = false;

  bool HasOpenedFsProtocols = false;
  bool HasOpenedGopProtocols = false;
  bool HasOpenedEdidProtocols = false;

  volatile efiFileInfo* KernelInfo = NULL;
  volatile void* Mmap = NULL;
  uint16 NumUsableMmapEntries = 0;
  usableMmapEntry* UsableMmap = NULL;



  // [Make sure the firmware-provided tables are valid]

  // Next, we want to check to see if the tables our firmware gave us are
  // even valid - and if they are, update their respective pointers (gST for
  // the System Table, gBS for Boot Services, and gRT for Runtime Services).

  // We can do this by checking their signature, as well as their size and
  // revision (since we require EFI 1.1+), like this:

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

  CommonInfoTable.Firmware.Efi.Tables.SystemTable.Pointer = SystemTable;
  gST = SystemTable;

  // (Check EFI Boot Services table, and update gBS)

  if (SystemTable->BootServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Signature != efiBootServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->BootServices->Hdr.Size < sizeof(efiBootServices)) {
    return EfiInvalidParameter;
  }

  CommonInfoTable.Firmware.Efi.Tables.BootServices.Pointer = gST->BootServices;
  gBS = gST->BootServices;

  // (Check EFI Runtime Services table, and update gRT)

  if (SystemTable->RuntimeServices == NULL) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Signature != efiRuntimeServicesSignature) {
    return EfiInvalidParameter;
  } else if (SystemTable->RuntimeServices->Hdr.Size < sizeof(efiRuntimeServices)) {
    return EfiInvalidParameter;
  }

  CommonInfoTable.Firmware.Efi.Tables.RuntimeServices.Pointer = gST->RuntimeServices;
  gRT = gST->RuntimeServices;




  // [Obtain the current CPU privilege level, and enable x64 features]

  // The EFI specification don't actually define what privilege level the
  // CPU is running in (at least, not before ExitBootServices()), so unlike
  // with BIOS, we don't actually have any guarantee that we're in ring 0.

  // That introduces a problem, because if we're not in ring 0, then we can't
  // execute any privileged instructions, which limits what we can do; so, in
  // order to guarantee we won't crash the system, we need to get the CPL.

  // (The bootloader will still run on any protection level, even ring 3,
  // but it won't be able to use all features unless it's in ring 0)

  uint8 Cpl = GetCpuProtectionLevel();
  CommonInfoTable.System.Cpu.x64.ProtectionLevel = Cpl;

  // Now that we're aware of the CPU privilege level, we can move onto the
  // next step, which is to make sure SSE (and other x64 features) are
  // enabled, since some firmware doesn't do that by default.

  // (This requires the system to be in ring 0 (supervisor mode))

  if (Cpl == 0) {

    // Before we do that though, let's save the current state of the
    // control registers:

    CommonInfoTable.System.Cpu.x64.Cr0 = ReadFromControlRegister(0);
    CommonInfoTable.System.Cpu.x64.Cr3 = ReadFromControlRegister(3);
    CommonInfoTable.System.Cpu.x64.Cr4 = ReadFromControlRegister(4);
    CommonInfoTable.System.Cpu.x64.Efer = ReadFromMsr(0xC0000080);

    // Now that we've saved the current state of the control registers, we
    // can move onto modifying them to enable the features we want.

    // (Enable the FPU, by setting the necessary bits in CR0)

    uint64 Cr0 = ReadFromControlRegister(0);

    Cr0 = setBit(Cr0, 1); // Set CR0.MP (1 << 1)
    Cr0 = setBit(Cr0, 4); // Set CR0.ET (1 << 4)
    Cr0 = setBit(Cr0, 5); // Set CR0.NE (1 << 5)

    WriteToControlRegister(0, Cr0);

    // (Enable SSE, by setting the necessary bits in CR4)

    uint64 Cr4 = ReadFromControlRegister(4);

    Cr4 = setBit(Cr4, 9); // Set CR4.OSFXSR (1 << 9)
    Cr4 = setBit(Cr4, 10); // Set CR4.OSXMMEXCPT (1 << 10)

    WriteToControlRegister(4, Cr4);

  }




  // [Check the availability of input/output protocols]

  // Next, we also want to check whether the input and output protocols (ConIn
  // and ConOut, respectively) are even supported.

  // Unlike the previous tables, these aren't guaranteed to be available on
  // every machine - for instance, most Macs don't support ConOut - so we
  // need to make sure they're actually available before we can use them.

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

  CommonInfoTable.Firmware.Efi.SupportsConIn = SupportsConIn;

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

  CommonInfoTable.Firmware.Efi.SupportsConOut = SupportsConOut;


  // [Switch to the best text mode available, if applicable]

  // Now that we know whether we can use EFI's text mode functions or not
  // (by checking whether ConOut is supported), we want to automatically
  // switch to the best text mode available.

  int32 ConOutMode = 0;
  uint32 ConOutResolution[2] = {0, 0};

  if (SupportsConOut == true) {

    // EFI mode numbers run between 0 and (MaxMode - 1), so the first
    // thing we need to do is look for MaxMode.

    int32 MaxMode = gST->ConOut->Mode->MaxMode;

    for (int32 Mode = 0; Mode < MaxMode; Mode++) {

      // (Query mode information; we want to filter for modes that are at
      // least 80*25 characters, and with the largest overall area)

      uint64 Columns, Rows;

      if (gST->ConOut->QueryMode(gST->ConOut, Mode, &Columns, &Rows) == EfiSuccess) {

        if ((Columns >= 80) && (Rows >= 25)) {

          // (If this text mode fits our criteria, then select it as our
          // best mode)

          if ((Columns * Rows) >= (ConOutResolution[0] * ConOutResolution[1])) {

            ConOutMode = Mode;

            ConOutResolution[0] = Columns;
            ConOutResolution[1] = Rows;

          }

        }

      }

    }

    // (Now that we know the best text mode available, let's switch to it)

    gST->ConOut->SetMode(gST->ConOut, ConOutMode);

    // (Lastly, let's fill out the common information table)

    CommonInfoTable.Display.Text.Format = Utf16Format;
    CommonInfoTable.Display.Text.LimitX = ConOutResolution[0];
    CommonInfoTable.Display.Text.LimitY = ConOutResolution[1];

  }

  // (Show a few initial messages, now that we've set up the console)

  Message(Boot, u"Successfully initialized the console.");

  Message(Info, u"This system's EFI revision is %d.%d", (uint64)((gST->Hdr.Revision >> 16) & 0xFF), (uint64)(gST->Hdr.Revision & 0xFF));
  Message(Info, u"This system is running in ring %d", (uint64)Cpl);
  Message(Info, u"Using text mode %d, with a %d*%d character resolution", ConOutMode, ConOutResolution[0], ConOutResolution[1]);

  if (Cpl > 0) {
    Message(Warning, u"System not running in ring 0; usability may be seriously limited.");
  }




  // [Check for GOP support]

  // Next, we also want to check for graphics mode support, since it's far
  // more useful than regular EFI text mode. (We won't be enabling it just
  // yet, though!)

  // Thankfully, this is actually pretty easy - we just need to check for
  // the existence of the GOP (Graphics Output Protocol), like this:

  Print(u"\n\r", 0);
  Message(Boot, u"Checking for UEFI GOP (Graphics Output Protocol) support.");

  efiHandle* GopHandles;
  uint64 NumGopHandles = 0;

  bool SupportsGop = false;

  // (Use LocateHandleBuffer() with `ByProtocol` to locate *every* handle
  // that supports efiGraphicsOutputProtocol)

  AppStatus = gBS->LocateHandleBuffer(ByProtocol, &efiGraphicsOutputProtocol_Uuid, NULL, &NumGopHandles, &GopHandles);

  if ((AppStatus == EfiSuccess) && (NumGopHandles > 0)) {

    Message(Ok, u"Located %d GOP handle(s) at %xh.", NumGopHandles, (uint64)GopHandles);
    HasOpenedGopHandles = true;

  }

  // (Open the first working handle that supports GOP; in theory every handle
  // returned by LocateHandleBuffer() *should* work, but you never know~)

  efiGraphicsOutputProtocol* GopProtocol = NULL;
  efiHandle GopProtocolHandle = GopHandles[0];

  for (uint64 HandleNum = 0; HandleNum < NumGopHandles; HandleNum++) {

    // (Initialize variables)

    GopProtocol = NULL;
    GopProtocolHandle = GopHandles[HandleNum];

    // (Try to open a efiGraphicsOutputProtocol instance from this handle)

    AppStatus = gBS->OpenProtocol(GopProtocolHandle, &efiGraphicsOutputProtocol_Uuid, (void**)&GopProtocol, ImageHandle, NULL, 1);

    if ((AppStatus == EfiSuccess) && (GopProtocol != NULL)) {

      HasOpenedGopProtocols = true;
      SupportsGop = true;

      Message(Ok, u"Located a valid GOP instance at handle %d.", HandleNum);
      break;

    } else {

      Message(Warning, u"Skipped invalid GOP handle %d.", HandleNum);

    }

  }

  // We also want to check for EDID (Extended Display Identification Data).
  // In theory, this should be supported on most systems, and provide us with
  // information about the screen we're connected to.

  // (Define EDID-related variables, as well as PreferredResolution[], which
  // is set to 1024 by 768 pixels by default.)

  efiEdidProtocol* EdidProtocol = NULL;
  uint16 PreferredResolution[2] = {1024, 768};

  bool SupportsEdid = false;

  if (SupportsGop == true) {

    // (Try to open an efiEdidActiveProtocol instance from this handle)

    AppStatus = gBS->OpenProtocol(GopProtocolHandle, &efiEdidActiveProtocol_Uuid, (void**)&EdidProtocol, ImageHandle, NULL, 1);

    if ((AppStatus == EfiSuccess) && (EdidProtocol != NULL)) {

      // (Even if there *is* a protocol attached to this handle, it may not
      // necessarily support EDID, so let's sanity-check our values first)

      HasOpenedEdidProtocols = true;

      if ((EdidProtocol->SizeOfEdid > 0) && (EdidProtocol->Edid != NULL)) {

        // Only *now* can we actually guarantee that EDID is supported,
        // which means we can fill out PreferredResolution.

        PreferredResolution[0] = (EdidProtocol->Edid[56] & 0xFF) | ((EdidProtocol->Edid[58] & 0xF0) << 4);
        PreferredResolution[1] = (EdidProtocol->Edid[59] & 0xFF) | ((EdidProtocol->Edid[61] & 0xF0) << 4);
        SupportsEdid = true;

        Message(Ok, u"Successfully located an EDID info block for the selected GOP protocol.");
        Message(Info, u"Preferred resolution appears to be %d*%d (according to EDID)",
               (uint64)PreferredResolution[0], (uint64)PreferredResolution[1]);

      } else {

        Message(Warning, u"GOP handle does not support EDID; could not find preferred resolution.");

      }

    } else {

      Message(Warning, u"GOP handle does not support EDID; could not find preferred resolution.");

    }

  }



  // [Find the best GOP mode available, if applicable]

  // If our system *does* support the Graphics Output Protocol, then the
  // next thing we want to do is try to find the best graphics mode
  // available on our system.

  // We do that by manually querying the firmware about each graphics
  // mode, like this (criteria further down):

  uint32 GopMode = 0;

  uint8 GopColorDepth = 0;
  uint32 GopResolution[2] = {0};

  if (SupportsGop == true) {

    Print(u"\n\r", 0);
    Message(Boot, u"Attempting to find the best graphics mode.");

    // Just like with EFI text mode, we also need to find MaxMode:

    uint32 MaxMode = GopProtocol->Mode->MaxMode;

    // Now that we know the mode limit, we can manually go through each
    // mode and try to find the best one, if possible.

    for (uint32 Mode = 0; Mode < MaxMode; Mode++) {

      volatile efiGraphicsOutputModeInformation* ModeInfo;
      uint64 Size;

      if (GopProtocol->QueryMode(GopProtocol, Mode, &Size, &ModeInfo) == EfiSuccess) {

        // (If the pixel format is PixelBltOnly, then that means it's not
        // a linear framebuffer, so make sure it's not that)

        if (ModeInfo->PixelFormat < PixelBltOnly) {

          // First, let's make sure the color depth is at least 16 bpp:

          uint8 ColorDepth = 24;

          if (ModeInfo->PixelFormat == PixelBitMask) {

            // (Calculate an accurate reserved mask; sometimes buggy firmware
            // will set `ModeInfo->PixelBitmask.ReservedMask` to zero.)

            uint32 ReservedMask = ~(ModeInfo->PixelBitmask.RedMask | ModeInfo->PixelBitmask.GreenMask | ModeInfo->PixelBitmask.BlueMask);
            ReservedMask |= ModeInfo->PixelBitmask.ReservedMask;

            // (Calculate the color depth, based off of that reserved mask.)

            ColorDepth = 32;

            for (int Bit = 0; Bit < 32; Bit++) {

              if ((ReservedMask & (1ULL << Bit)) != 0) {
                ColorDepth--;
              }

            }

          }

          if (ColorDepth < 16) {

            Message(Warning, u"Skipping GOP mode %d (color depth is too low)");
            continue;

          }

          // Okay - now that we know the color depth isn't too low, we want
          // to check for the following things, in order:

          const uint32 HRes = ModeInfo->HorizontalResolution;
          const uint32 VRes = ModeInfo->VerticalResolution;

          // (1) Whether this mode's resolution is at least 640 by 480;

          if ((HRes >= 640) && (VRes >= 480)) {

            // (2) Whether this mode's resolution is within the bounds of
            // PreferredResolution[];

            if ((HRes <= PreferredResolution[0]) && (VRes <= PreferredResolution[1])) {

              // (3) Whether this mode's color depth is above the current
              // maximum;

              if (ColorDepth >= GopColorDepth) {

                // (4) Whether this mode's *area* is above or equal to the
                // current maximum.

                if ((HRes * VRes) >= (GopResolution[0] * GopResolution[1])) {

                  GopColorDepth = ColorDepth;
                  GopMode = Mode;
                  GopResolution[0] = ModeInfo->HorizontalResolution;
                  GopResolution[1] = ModeInfo->VerticalResolution;

                }

              }

            }

          }

        } else {

          Message(Warning, u"Skipping GOP mode %d (unsupported pixel format %d).", Mode, ModeInfo->PixelFormat);
          continue;

        }

      }

    }

    // (If we *did* actually find a valid mode, then show the mode number,
    // as well as the resolution and color depth - otherwise, error.)

    if ((GopResolution[0] == 0) || (GopResolution[1] == 0)) {

      Message(Fail, u"Couldn't find a graphics mode that met the necessary criteria.");
      SupportsGop = false; SupportsEdid = false;

    } else {

      Message(Ok, u"Found GOP mode %d, with a %d*%d resolution and %d-bit color depth.",
              (uint64)GopMode, (uint64)GopResolution[0], (uint64)GopResolution[1], (uint64)GopColorDepth);

    }

  }

  // (Update the common information tables)

  if (SupportsGop == true) {

    if (SupportsEdid == true) {

      CommonInfoTable.Display.Edid.IsSupported = true;
      CommonInfoTable.Display.Edid.PreferredResolution[0] = PreferredResolution[0];
      CommonInfoTable.Display.Edid.PreferredResolution[1] = PreferredResolution[1];
      CommonInfoTable.Display.Edid.Table.Pointer = (void*)EdidProtocol->Edid;

    }

    CommonInfoTable.Display.Type = GopDisplay;
    CommonInfoTable.Display.Graphics.LimitX = GopResolution[0];
    CommonInfoTable.Display.Graphics.LimitY = GopResolution[1];

    CommonInfoTable.Firmware.Efi.Gop.IsSupported = true;
    CommonInfoTable.Firmware.Efi.Gop.Protocol.Pointer = GopProtocol;

  } else if (SupportsConOut == true) {

    CommonInfoTable.Display.Type = EfiTextDisplay;

  } else {

    CommonInfoTable.Display.Type = UnknownDisplay;

  }




  // [Find ACPI and SMBIOS tables]

  // We won't need these for now, but this *does* end up being pretty useful
  // for our kernel later on; for example, ACPI helps with power management,
  // device initialization, etc.

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to find ACPI and SMBIOS tables.");

  // (Prepare initial variables)

  void* AcpiTable = NULL;
  void* SmbiosTable = NULL;

  bool SupportsAcpi = false;
  bool SupportsNewAcpi = false;

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

  // (Fill out the common information table, and show information - for
  // both ACPI and SMBIOS.)

  if ((SupportsAcpi == true) || (SupportsNewAcpi == true)) {

    CommonInfoTable.System.Acpi.IsSupported = true;
    CommonInfoTable.System.Acpi.Table.Pointer = AcpiTable;

    Message(Ok, u"ACPI appears to be present; using table at %xh.", (uint64)AcpiTable);

  } else {

    CommonInfoTable.System.Acpi.IsSupported = false;
    Message(Warning, u"ACPI appears to be unsupported.");

  }

  if ((SupportsSmbios == true) || (SupportsNewSmbios == true)) {

    CommonInfoTable.System.Smbios.IsSupported = true;
    CommonInfoTable.System.Smbios.Table.Pointer = SmbiosTable;

    Message(Ok, u"SMBIOS appears to be present; using table at %xh.", (uint64)SmbiosTable);

  } else {

    CommonInfoTable.System.Smbios.IsSupported = false;
    Message(Warning, u"SMBIOS appears to be unsupported.");

  }




  // [Initialize filesystem protocols]

  // The next thing we want to do is to load the kernel from disk. Before
  // we can do that though, we need to be able to read from the *filesystem*
  // on that disk.

  // Thankfully, EFI provides a (relatively) easy and standardized way to do
  // this: the Simple Filesystem Protocol (as well as the File Protocol).

  // All we need to do is to look through each instance/handle of that
  // protocol, and find the one that has the kernel image in it.

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to initialize filesystem protocols.");

  // (Use LocateHandleBuffer() with `ByProtocol` to locate every handle
  // that supports efiSimpleFilesystemProtocol)

  efiHandle* FsHandles = NULL;
  uint64 NumFsHandles = 0;

  AppStatus = gBS->LocateHandleBuffer(ByProtocol, &efiSimpleFilesystemProtocol_Uuid, NULL, &NumFsHandles, &FsHandles);

  if ((AppStatus != EfiSuccess) || (NumFsHandles == 0)) {

    Message(Error, u"Failed to locate any filesystem protocol handles.");
    goto ExitEfiApplication;

  } else {

    Message(Ok, u"Located %d filesystem protocol handle(s) at %xh.", NumFsHandles, (uint64)FsHandles);
    HasOpenedFsHandles = true;

  }

  // (Go through each handle, open its protocol, and check if it has the
  // kernel image in it; if it does, save it as FsProtocol/KernelFileHandle.)

  efiSimpleFilesystemProtocol* FsProtocol = NULL;
  volatile efiHandle FsProtocolHandle = NULL;

  efiFileProtocol* KernelFileHandle = NULL;

  for (uint64 HandleNum = 0; HandleNum < NumFsHandles; HandleNum++) {

    // (Initialize variables)

    FsProtocol = NULL;
    FsProtocolHandle = NULL;
    efiFileProtocol* FileProtocol = NULL;

    // (Open this handle's protocol, as FsProtocol)

    AppStatus = gBS->OpenProtocol(FsHandles[HandleNum], &efiSimpleFilesystemProtocol_Uuid, (void**)&FsProtocol, ImageHandle, NULL, 1);

    if ((AppStatus == EfiSuccess) && (FsProtocol != NULL)) {

      FsProtocolHandle = FsHandles[HandleNum];
      HasOpenedFsProtocols = true;

      Message(Info, u"Successfully opened protocol for FsHandle[%d] at %xh", HandleNum, (uint64)FsProtocol);

      // (Open FsProtocol's filesystem volume)

      AppStatus = FsProtocol->OpenVolume(FsProtocol, (void**)&FileProtocol);

      if ((AppStatus == EfiSuccess) && (FileProtocol != NULL)) {

        // (Open a efiFileProtocol handle at the kernel image location)

        AppStatus = FileProtocol->Open(FileProtocol, (void**)&KernelFileHandle, KernelLocation, 1, 0);

        // (If it worked, then that means the image is there, so let's save
        // it in KernelFileHandle and leave this entire loop)

        if ((AppStatus == EfiSuccess) && (KernelFileHandle != NULL)) {

          Message(Ok, u"Successfully found kernel image (at %s).", KernelLocation);
          Message(Info, u"Kernel file protocol handle is located at %xh", (uint64)KernelFileHandle);

          break;

        }

      }

      gBS->CloseProtocol(FsHandles[HandleNum], &efiSimpleFilesystemProtocol_Uuid, ImageHandle, NULL);

    }

    HasOpenedFsProtocols = false;

  }

  // (Check if we managed to find the kernel image, and update common
  // information tables)

  if ((HasOpenedFsProtocols == false) || (FsProtocolHandle == NULL)) {

    Message(Error, u"Failed to locate the kernel image (should be at %s).", KernelLocation);
    goto ExitEfiApplication;

  }

  CommonInfoTable.Disk.AccessMethod = EfiFsMethod;

  CommonInfoTable.Disk.EfiFs.HandleList.Pointer = (void*)FsHandles;
  CommonInfoTable.Disk.EfiFs.Handle.Pointer = (void*)KernelFileHandle;
  CommonInfoTable.Disk.EfiFs.Protocol.Pointer = (void*)FsProtocol;



  // [Read the kernel image from disk]

  // Now that we've obtained the kernel image's file handle, the next thing
  // we need to do is to read it from disk.

  // This is a little more involved though; before we can actually read it,
  // we first need to figure out how large it is and then allocate the
  // necessary buffer, which can take some time.

  uint64 KernelSize = 0;

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to read the kernel image from disk.");

  // (Our kernel's file handle has an information table of its own (defined
  // in efiFileInfo{}), which we need to get the size of.)

  KernelInfo = NULL;
  uint64 KernelInfoSize = 0;

  AppStatus = KernelFileHandle->GetInfo(KernelFileHandle, &efiFileInfo_Uuid, &KernelInfoSize, KernelInfo);

  if ((AppStatus != EfiBufferTooSmall) || (KernelInfoSize == 0)) {

    Message(Error, u"Failed to obtain the size of the efiFileInfo table.");
    goto ExitEfiApplication;

  } else {

    // In theory, GetInfo() should have updated KernelInfoSize with the
    // actual size of the table, so now we can properly allocate a
    // buffer for it.

    Message(Info, u"Size of the efiFileInfo table is %d bytes", KernelInfoSize);
    AppStatus = gBS->AllocatePool(EfiLoaderData, KernelInfoSize, (volatile void**)&KernelInfo);

    if (AppStatus == EfiSuccess) {

      Memset((void*)KernelInfo, 0, KernelInfoSize);

    } else {

      Message(Error, u"Failed to allocate space for the efiFileInfo table.");
      goto ExitEfiApplication;

    }

    // (Now that we have a proper buffer ready, call GetInfo() again.)

    AppStatus = KernelFileHandle->GetInfo(KernelFileHandle, &efiFileInfo_Uuid, &KernelInfoSize, KernelInfo);

    if ((AppStatus != EfiSuccess) || (KernelInfo == NULL)) {

      Message(Error, u"Failed to obtain the kernel's EFI file info table.");
      goto ExitEfiApplication;

    } else {

      CommonInfoTable.Disk.EfiFs.FileInfo.Pointer = (void*)KernelInfo;

      Message(Ok, u"Successfully obtained kernel's EFI file info table.");
      Message(Info, u"efiFileInfo table is located at %xh", (uint64)KernelInfo);

    }

  }

  // Now that we have the kernel file handle's information table, we can look
  // through it to find the correct size. (We also sanity-check the data
  // just to be sure)

  if (KernelInfo->FileSize != 0) {

    KernelSize = KernelInfo->FileSize;

  } else if (KernelInfo->PhysicalSize != 0) {

    KernelSize = KernelInfo->PhysicalSize;

  } else {

    Message(Error, u"Kernel image size (returned by efiFileProtocol->GetInfo()) is zero.");
    goto ExitEfiApplication;

  }

  Message(Info, u"Kernel image size is %d bytes.", KernelSize);

  // Finally, now that we actually know the size of the kernel image, we
  // can allocate space for it, and read it from disk.

  #define AlignDivision(Num, Divisor) ((Num + Divisor - 1) / Divisor)

  AppStatus = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, AlignDivision(KernelSize, 4096), (volatile efiPhysicalAddress*)&Kernel);

  if ((AppStatus != EfiSuccess) || (Kernel == NULL)) {

    Message(Error, u"Failed to allocate enough space (%d KiB) for the kernel.", AlignDivision(KernelSize, 1024));
    goto ExitEfiApplication;

  } else {

    Message(Ok, u"Successfully allocated %d KiB of space for the kernel.", AlignDivision(KernelSize, 1024));
    HasAllocatedKernel = true;

  }

  // (Read the kernel image into our newly allocated buffer, at *Kernel)

  AppStatus = KernelFileHandle->Read(KernelFileHandle, &KernelSize, Kernel);

  if ((AppStatus != EfiSuccess) || (Kernel == NULL)) {

    Message(Error, u"Failed to read the kernel image from disk.");
    goto ExitEfiApplication;

  } else {

    Message(Ok, u"Successfully loaded kernel to %xh in memory.", (uint64)Kernel);

  }



  // [Allocate space for the kernel stack]

  // Apart from the kernel, we also need to allocate space for the kernel
  // stack (which is different from the firmware-provided one); thankfully,
  // this is a lot easier.

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to allocate space for the kernel stack.");

  // (Initialize variables, and allocate as many pages as necessary for
  // *at least* KernelStackSize bytes.)

  KernelStack = NULL;
  AppStatus = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, AlignDivision(KernelStackSize, 4096), (volatile efiPhysicalAddress*)&KernelStack);

  if ((AppStatus != EfiSuccess) || (KernelStack == NULL)) {

    Message(Error, u"Failed to allocate enough space for the kernel (%d MiB)", (KernelSize / 1048576));
    goto ExitEfiApplication;

  } else {

    Message(Ok, u"Successfully allocated %d KiB of space for the kernel stack.", (KernelStackSize / 1024));
    HasAllocatedStack = true;

  }

  // (Define KernelStackTop, and update the kernel information table)

  void* KernelStackTop = (void*)((uint64)KernelStack + KernelStackSize);

  CommonInfoTable.Image.StackSize = AlignDivision(KernelStackSize, 4096);
  CommonInfoTable.Image.StackTop.Pointer = KernelStackTop;



  // [Read kernel ELF headers]

  // Like our EFI bootloader, our kernel is stored as an executable file, not
  // as a raw binary - however, unlike an EFI application, it uses the ELF
  // format, *not* PE.

  // That means we need to manually parse the file headers in order to figure
  // out important details (like where it should be loaded, or whether it
  // even has the right architecture).

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to read the kernel's executable headers.");

  // Our first step is to validate the ELF headers - we'll be running the
  // same checks here as we did in the BIOS loader.

  elfHeader* KernelHeader = (elfHeader*)Kernel;
  bool KernelHasValidElf = true;

  if (KernelHeader->Ident.MagicNumber != 0x464C457F) {
    KernelHasValidElf = false;
  } else if ((KernelHeader->Ident.Class != 2) || (KernelHeader->MachineType != 0x3E)) {
    KernelHasValidElf = false;
  } else if ((KernelHeader->SectionHeaderOffset == 0) || (KernelHeader->NumSectionHeaders == 0)) {
    KernelHasValidElf = false;
  } else if (KernelHeader->StringSectionIndex == 0) {
    KernelHasValidElf = false;
  } else if ((KernelHeader->ProgramHeaderOffset == 0) || (KernelHeader->NumProgramHeaders == 0)) {
    Message(Warning, u"Kernel does not appear to have any ELF program headers.");
  } else if (KernelHeader->Version < 1) {
    Message(Warning, u"ELF header version appears to be invalid (%d)", (uint64)KernelHeader->Version);
  } else if (KernelHeader->FileType != 2) {
    Message(Warning, u"ELF header appears to have a non-executable file type (%d)", (uint64)KernelHeader->FileType);
  }

  // (Show messages, and debug information)

  #define NumProgramHdrs (uint64)KernelHeader->NumProgramHeaders
  #define NumSectionHdrs (uint64)KernelHeader->NumSectionHeaders

  #define ProgramHdrOffset KernelHeader->ProgramHeaderOffset
  #define SectionHdrOffset KernelHeader->SectionHeaderOffset

  if (KernelHasValidElf == false) {

    Message(Error, u"Kernel does not appear to be a valid ELF executable.");

    AppStatus = EfiLoadError;
    goto ExitEfiApplication;

  } else {

    Message(Ok, u"Kernel appears to be a valid ELF executable.");
    Message(Info, u"ELF header is located at %xh", (uint64)KernelHeader);
    Message(Info, u"ELF entrypoint is located at +%xh", KernelHeader->Entrypoint);

    Message(Info, u"Found %d program header(s), starting at +%xh.", NumProgramHdrs, ProgramHdrOffset);
    Message(Info, u"Found %d section header(s), starting at +%xh.", NumSectionHdrs, SectionHdrOffset);

  }

  // Next, we want to find the .text section, since that's where our kernel's
  // executable code is located. Before we do that though, we need to go
  // through the string section, like this:

  #define ProgramHdrSize KernelHeader->ProgramHeaderSize
  #define SectionHdrSize KernelHeader->SectionHeaderSize

  #define GetProgramHeader(Start, Index) (elfProgramHeader*)(Start + ProgramHdrOffset + (Index * ProgramHdrSize))
  #define GetSectionHeader(Start, Index) (elfSectionHeader*)(Start + SectionHdrOffset + (Index * SectionHdrSize))
  #define GetSectionString(Start, SectionHeader, StringHeader) (const char8*)(Start + StringHeader->Offset + SectionHeader->NameOffset)

  elfSectionHeader* KernelStringSection = GetSectionHeader((uint64)KernelHeader, KernelHeader->StringSectionIndex);
  elfSectionHeader* KernelTextSection = NULL;

  for (uint64 Index = 1; Index < NumSectionHdrs; Index++) {

    // (Get the section string (the name string) of the current section)

    elfSectionHeader* Section = GetSectionHeader((uint64)KernelHeader, Index);
    const char8* SectionString = GetSectionString((uint64)KernelHeader, Section, KernelStringSection);

    // (Compare the name string against u8".text"; if it matches, then we've
    // successfully found the kernel's executable section!)

    if (StrcmpShort(SectionString, u8".text") == true) {

      Message(Ok, u"Successfully located the kernel header's executable section.");
      Message(Info, u"Section header is located at %xh", (uint64)Section);
      Message(Info, u"Section starts at +%xh and is %d bytes long", Section->Offset, Section->Size);

      KernelTextSection = Section;
      break;

    }

  }

  // (If we couldn't find the .text section, then panic)

  if (KernelTextSection == NULL) {

    Message(Error, u"Couldn't locate the kernel file's .text section.");

    AppStatus = EfiLoadError;
    goto ExitEfiApplication;

  }

  // Finally, now that we know which section is the .text section, we can
  // calculate the real kernel entrypoint.

  KernelEntrypoint = (void*)((uint64)Kernel + KernelTextSection->Offset + KernelHeader->Entrypoint);

  Message(Ok, u"Successfully located actual kernel entrypoint.");
  Message(Info, u"Kernel entrypoint is at %xh", (uint64)KernelEntrypoint);

  // (Fill out the common information table)

  CommonInfoTable.Image.Type = ElfImageType;
  CommonInfoTable.Image.Executable.Entrypoint.Pointer = KernelEntrypoint;
  CommonInfoTable.Image.Executable.Header.Pointer = KernelHeader;




  // [Obtain the system memory map]

  // Now that we've allocated everything we need, we can move onto the next
  // step: memory management.

  // Our kernel needs to know about the system's memory map, so it knows
  // which pages it can use - thankfully though, EFI actually makes this
  // kind of a breeze, with gBS->GetMemoryMap().

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to obtain the system's EFI memory map.");

  // (Declare initial variables - we'll be calling GetMemoryMap() further
  // ahead, so to avoid issues with Mmap == NULL, we point it to a
  // temporary 64-byte buffer instead.)

  Mmap = NULL;
  UsableMmap = NULL;

  uint64 MmapKey;
  volatile uint64 MmapSize = 0;

  volatile uint64 MmapDescriptorSize = 0;
  volatile uint32 MmapDescriptorVersion = 0;

  // (We'll be calling GetMemoryMap() further ahead, so to avoid any potential
  // problems with Mmap == NULL, we point it to a temporary buffer for now)

  uint8 MmapTemp[64];
  Mmap = MmapTemp;

  // The first step is to call GetMemoryMap(), with MmapSize == 0; since
  // the memory map can't fit in zero bytes, this will return an error.

  // Crucially though, it'll *also* update MmapSize to contain the actual
  // size of the memory map, which we can then allocate.

  AppStatus = gBS->GetMemoryMap(&MmapSize, Mmap, &MmapKey, &MmapDescriptorSize, &MmapDescriptorVersion);

  if (AppStatus != EfiBufferTooSmall) {

    if ((MmapSize != 0) && (MmapDescriptorSize != 0)) {

      Message(Warning, u"Initial call to GetMemoryMap() appears to be badly implemented.");

    } else {

      Message(Fail, u"GetMemoryMap() appears to be working incorrectly.");
      goto ExitEfiApplication;

    }

  }

  Message(Ok, u"Initial call to GetMemoryMap() indicates memory map is %d bytes long.", MmapSize);

  // Now that we know the size of the memory map, we want to allocate a
  // buffer for it, with some margin for error (in this case, up to four
  // additional entries).

  // (We also do the same for the *usable* memory map, which we'll fill
  // out later on.)

  MmapSize += (MmapDescriptorSize * 4);
  AppStatus = gBS->AllocatePool(EfiLoaderData, MmapSize, (volatile void**)&Mmap);

  if (AppStatus == EfiSuccess) {

    // (Show a success message)

    Message(Ok, u"Successfully allocated a %d-byte buffer for the memory map.", MmapSize);
    Message(Info, u"Memory map buffer is located at %xh", (uint64)Mmap);

    // (Repeat the same process, but for the usable memory map.)

    uint64 UsableMmapSize = (MmapSize * sizeof(usableMmapEntry) / MmapDescriptorSize);
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

  // Now that we have a suitable buffer for the regular memory map, we can
  // call GetMemoryMap() a second time to actually fill it out.

  AppStatus = gBS->GetMemoryMap(&MmapSize, Mmap, &MmapKey, &MmapDescriptorSize, &MmapDescriptorVersion);

  if (AppStatus == EfiSuccess) {

    Message(Ok, u"Successfully obtained the system memory map.");
    Message(Info, u"There are %d memory map entries, which are %d bytes each",
            (MmapSize / MmapDescriptorSize), MmapDescriptorSize);

  } else {

    Message(Fail, u"Failed to obtain the system memory map.");
    goto ExitEfiApplication;

  }



  // [Process the system memory map]

  // Now that we have the system's memory map, we have a pretty good idea of
  // which areas we can use, and which ones we can't.

  // However, there's nothing that actually guarantees it's been cleaned
  // up; not only is it often unsorted, but there can sometimes even be
  // overlapping entries!

  // The purpose of this section is, essentially, to be able to deal with
  // that - not only do we "clean up" the regular memory map, but we also
  // build a 'usable' memory map that we can later pass onto the kernel.

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to process the system memory map.");

  // (Make sure NumMmapEntries can fit into a 16-bit integer)

  uint64 NumMmapEntries = (MmapSize / MmapDescriptorSize);

  if (NumMmapEntries >= 65536) {

    Message(Warning, u"Too many memory map entries - limiting to 65536.");
    NumMmapEntries = 65535;

  }

  // (Sort each entry by their starting address)

  // Since we can't dynamically allocate memory without changing the
  // memory map, we need a sorting algorithm that uses no additional
  // space, like insertion sort:

  for (uint16 Threshold = 1; Threshold < NumMmapEntries; Threshold++) {

    for (uint16 Position = Threshold; Position > 0; Position--) {

      // (Get the actual entries themselves)

      efiMemoryDescriptor* Entry = GetMmapEntry(Mmap, MmapDescriptorSize, Position);
      efiMemoryDescriptor* PreviousEntry = GetMmapEntry(Mmap, MmapDescriptorSize, (Position - 1));

      // (Use insertion sort to sort the entries; while Mmap[n] < Mmap[n-1],
      // swap them both, and decrement n.)

      if (Entry->PhysicalStart < PreviousEntry->PhysicalStart) {
        Memswap((void*)Entry, (void*)PreviousEntry, MmapDescriptorSize);
      }

      // (If we have two entries with the same starting point, make sure
      // that the 'usable' one always comes last)

      if (Entry->PhysicalStart == PreviousEntry->PhysicalStart) {

        if (PreviousEntry->Type == EfiConventionalMemory) {
          Memswap((void*)Entry, (void*)PreviousEntry, MmapDescriptorSize);
        }

      }

    }

  }

  // Now that we've sorted the memory map, we can move onto the next part:
  // actually building the usable memory map.

  // (We also want to set aside a certain amount of memory for the
  // firmware to use, defined in MinFirmwareMemory.)

  NumUsableMmapEntries = 0;
  uint64 UsableMemory = 0;
  uint64 FirmwareMemory = MinFirmwareMemory;

  uint64 UsableOffset = 0;

  // In this case, the usable memory map is simply just a 'simplified'
  // memory map that accounts for overlapping and duplicate entries, and
  // that *only* contains memory that the kernel can use.

  // Since we don't plan on calling ExitBootServices(), that means we're
  // still running 'alongside' the firmware - that means we need to set
  // aside some space for it, as well as explicitly allocate any memory
  // meant for the kernel.

  uint16 MmapPositionThreshold = 0;
  uint64 UsableMmapThreshold = 0;

  for (uint16 Position = 0; Position < NumMmapEntries; Position++) {

    // (Find the first entry that starts on or at UsableMmapThreshold, of
    // type EfiConventionalMemory)

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

    // (Now that we've found it, compare it with the starting position of
    // the next entry and update the entry size if the two overlap)

    uint64 Start = Entry->PhysicalStart;
    uint64 Size = (Entry->NumberOfPages * 4096);

    if (MmapPositionThreshold < NumMmapEntries) {

      efiMemoryDescriptor* NextEntry = GetMmapEntry(Mmap, MmapDescriptorSize, MmapPositionThreshold);

      if (NextEntry->PhysicalStart < (Start + Size)) {

        Entry->NumberOfPages = (NextEntry->PhysicalStart - Start) / 4096;
        Size = (NextEntry->PhysicalStart - Start);

      }

    }

    // (Make sure we have enough memory for the firmware, as defined in
    // MinFirmwareMemory (see Bootloader.h))

    if (FirmwareMemory > 0) {

      if (Size < FirmwareMemory) {

        Message(Info, u"Reserved %d KiB for the firmware between %xh and %xh",
                (Size / 1024), Start, (Start + Size));

        FirmwareMemory -= Size;
        continue;

      } else {

        Message(Info, u"Reserved %d KiB for the firmware between %xh and %xh",
                (FirmwareMemory / 1024), Start, (Start + FirmwareMemory));

        Start += FirmwareMemory;
        Size -= FirmwareMemory;

        FirmwareMemory = 0;
        UsableOffset = Start;

      }

    }

    // (Align everything to 4 KiB (page) boundaries, and make sure each
    // entry isn't empty)

    if ((Start % 4096) != 0) {

      uint16 Remainder = (Start % 4096);

      Size -= (4096 - Remainder);
      Start += (4096 - Remainder);

    }

    Size -= (Size % 4096);

    if (Size == 0) {
      continue;
    }

    // Finally, we now have a memory area that we know is guaranteed to be
    // usable, so let's allocate it (as `EfiLoaderData`) and add it to
    // UsableMmap.

    AppStatus = gBS->AllocatePages(AllocateAddress, EfiLoaderData, (Size / 4096), (volatile efiPhysicalAddress*)&Start);

    if (AppStatus == EfiSuccess) {

      UsableMmap[NumUsableMmapEntries].Base = Start;
      UsableMmap[NumUsableMmapEntries].Limit = Size;
      UsableMemory += Size;

      Message(Info, u"Allocated %d KiB for the kernel between %xh and %xh",
              (Size / 1024), Start, (Start + Size));

      NumUsableMmapEntries++;

    } else {

      Message(Warning, u"Failed to allocate space for the kernel between %xh and %xh.",
              Start, (Start + Size));

      continue;

    }

  }

  // (Sanity-check the output, and show infomation.)

  if (NumUsableMmapEntries == 0) {

    Message(Fail, u"Unable to process the system memory map; out of resources.");

    AppStatus = EfiOutOfResources;
    goto ExitEfiApplication;

  } else {

    HasAllocatedMemory = true;

    Message(Ok, u"Allocated %d MiB of space (over %d usable memory area(s)) for the kernel.",
            (UsableMemory / 1048576), (uint64)NumUsableMmapEntries);

    Message(Info, u"Usable memory below %xh is reserved for the firmware", UsableOffset);

  }

  // (Fill out the common information table with the necessary values)

  CommonInfoTable.Firmware.Efi.Mmap.NumEntries = NumMmapEntries;
  CommonInfoTable.Firmware.Efi.Mmap.EntrySize = MmapDescriptorSize;
  CommonInfoTable.Firmware.Efi.Mmap.List.Pointer = (void*)Mmap;

  CommonInfoTable.Memory.NumEntries = NumUsableMmapEntries;
  CommonInfoTable.Memory.List.Pointer = (void*)UsableMmap;
  CommonInfoTable.Memory.PreserveUntilOffset = UsableOffset;




  // [Transfer control to the kernel]

  // Now that we've successfully initialized everything - including the
  // kernel info table - we can transfer control to the kernel itself.

  Print(u"\n\r", 0);
  Message(Boot, u"Preparing to transfer control to the kernel.");

  Message(Info, u"(function) TransitionStub() is at %xh", (uint64)(&TransitionStub));
  Message(Info, u"(commonInfoTable*) CommonInfoTable is at %xh", (uint64)(&CommonInfoTable));
  Message(Info, u"(void*) KernelEntrypoint is at %xh", (uint64)KernelEntrypoint);
  Message(Info, u"(void*) KernelStackTop is at %xh", (uint64)KernelStackTop);

  // (If GOP is enabled, enable a graphics mode, and fill out information)

  if (SupportsGop == true) {

    Message(Boot, u"Enabling graphics mode %d", (uint64)GopMode);
    AppStatus = GopProtocol->SetMode(GopProtocol, GopMode);

    if (AppStatus == EfiSuccess) {

      // Get the mode information, calculate the pixel size, and fill out
      // the color mask information.

      efiGraphicsOutputModeInformation* ModeInfo = GopProtocol->Mode->Info;
      uint8 PixelSize = 32;

      if (ModeInfo->PixelFormat == PixelBitMask) {

        // (Calculate the pixel size manually)

        uint32 CombinedMask = ModeInfo->PixelBitmask.RedMask;
        CombinedMask |= ModeInfo->PixelBitmask.GreenMask;
        CombinedMask |= ModeInfo->PixelBitmask.BlueMask;
        CombinedMask |= ModeInfo->PixelBitmask.ReservedMask;

        uint32 Bit = 32;

        do {

          Bit--;
          if ((CombinedMask & (1UL << Bit)) != 0) break;

        } while (Bit > 0);

        // (Show the bitmasks)

        CommonInfoTable.Display.Graphics.Bits.RedMask = ModeInfo->PixelBitmask.RedMask;
        CommonInfoTable.Display.Graphics.Bits.GreenMask = ModeInfo->PixelBitmask.GreenMask;
        CommonInfoTable.Display.Graphics.Bits.BlueMask = ModeInfo->PixelBitmask.BlueMask;

      } else if (ModeInfo->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {

        CommonInfoTable.Display.Graphics.Bits.RedMask = 0xFF;
        CommonInfoTable.Display.Graphics.Bits.GreenMask = 0xFF00;
        CommonInfoTable.Display.Graphics.Bits.BlueMask = 0xFF0000;

      } else if (ModeInfo->PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {

        CommonInfoTable.Display.Graphics.Bits.RedMask = 0xFF0000;
        CommonInfoTable.Display.Graphics.Bits.GreenMask = 0xFF00;
        CommonInfoTable.Display.Graphics.Bits.BlueMask = 0xFF;

      }

      CommonInfoTable.Display.Graphics.Bits.PerPixel = PixelSize;

      // Fill out the rest of the common information table.

      CommonInfoTable.Display.Graphics.Framebuffer.Address = (uint64)(GopProtocol->Mode->FramebufferBase);
      CommonInfoTable.Display.Graphics.Pitch = (GopProtocol->Mode->Info->PixelsPerScanline * AlignDivision(PixelSize, 8));
      CommonInfoTable.Display.Graphics.LimitX = ModeInfo->HorizontalResolution;
      CommonInfoTable.Display.Graphics.LimitY = ModeInfo->VerticalResolution;

    } else {

      CommonInfoTable.Display.Type = ((SupportsConOut == true) ? EfiTextDisplay : UnknownDisplay);
      SupportsGop = false;

      Message(Fail, u"Failed to enable default graphics mode; staying with EFI text mode.");

    }

  }

  // (Calculate the table checksum - we do this by manually summing every
  // byte in the table, up to the Checksum element)

  uint16 ChecksumSize = CommonInfoTable.Size - sizeof(CommonInfoTable.Checksum);
  uint8* RawCommonInfoTable = (uint8*)(&CommonInfoTable);

  for (uint16 Offset = 0; Offset < ChecksumSize; Offset++) {
    CommonInfoTable.Checksum += RawCommonInfoTable[Offset];
  }

  // Finally, transfer control to the kernel - we keep track of the return
  // status, in case something goes wrong

  uint64 KernelStatus = TransitionStub(&CommonInfoTable, KernelEntrypoint, KernelStackTop);

  // If the kernel *does* return, then we need to show the return status,
  // like this:

  // (If it was enabled, then disable graphics mode)

  if (SupportsGop == true) {

    GopProtocol->SetMode(GopProtocol, 0);

    if (SupportsConOut == true) {
      gST->ConOut->SetMode(gST->ConOut, 0);

    }

  }

  // (Show message, and exit EFI application)

  DebugFlag = true;
  Message(Error, u"Failed to load the kernel entrypoint.");
  Message(Info, u"Entrypoint returned with a status code of %xh", KernelStatus);

  AppStatus = KernelStatus;
  goto ExitEfiApplication;




  // ---------------- Restore state and exit EFI application ----------------

  ExitEfiApplication:

    // If possible, wait for the user to strike a key, and only then
    // continue with the process.

    Print(u"\n\r", 0);

    if ((SupportsConIn == true) && (SupportsConOut == true)) {

      Message(Warning, u"Press any key to return.");
      while (gST->ConIn->ReadKeyStroke(gST->ConIn, &PhantomKey) == EfiNotReady);

    }

    // If text mode / ConOut is enabled, then restore regular text mode
    // (which is always mode 0)

    if (SupportsConOut == true) {
      gST->ConOut->SetMode(gST->ConOut, 0);
    }

    // If we've allocated memory *for the usable memory map*, then free it.

    if (HasAllocatedMemory == true) {

      for (uint16 Entry = 0; Entry < NumUsableMmapEntries; Entry++) {
        gBS->FreePages(UsableMmap[Entry].Base, (UsableMmap[Entry].Limit / 4096));
      }

    }

    // If we've allocated memory *for the kernel*, then free it.

    if (HasAllocatedKernel == true) {
      gBS->FreePages((efiPhysicalAddress)Kernel, AlignDivision(KernelSize, 4096));
    }

    // If we've allocated memory *for the kernel stack*, then free it.

    if (HasAllocatedStack == true) {
      gBS->FreePages((efiPhysicalAddress)KernelStack, AlignDivision(KernelStackSize, 4096));
    }

    // If we've opened file system handles/protocols, then free/close them.

    if (HasOpenedFsHandles == true) {

      if (HasOpenedFsProtocols == true) {
        gBS->CloseProtocol(FsProtocolHandle, &efiSimpleFilesystemProtocol_Uuid, ImageHandle, NULL);
      }

      gBS->FreePool(FsHandles);

    }

    // If we've opened GOP handles/protocols, then free/close them.

    if (HasOpenedGopHandles == true) {

      if (HasOpenedGopProtocols == true) {

        if (HasOpenedEdidProtocols == true) {
          gBS->CloseProtocol(GopProtocolHandle, &efiEdidActiveProtocol_Uuid, ImageHandle, NULL);
        }

        gBS->CloseProtocol(GopProtocolHandle, &efiGraphicsOutputProtocol_Uuid, ImageHandle, NULL);
      }

      gBS->FreePool(GopHandles);

    }

    // If we've allocated memory pools, then free them.

    #define FreePoolIfAllocated(Ptr) if (Ptr != NULL) gBS->FreePool((void*)Ptr);

    FreePoolIfAllocated(KernelInfo);
    FreePoolIfAllocated(Mmap);
    FreePoolIfAllocated(UsableMmap);

    // If we're in ring 0, then restore the initial values of the CR0 and
    // CR4 registers, since we modified them early on.

    if (Cpl == 0) {

      WriteToControlRegister(0, CommonInfoTable.System.Cpu.x64.Cr0); // Restore CR0
      WriteToControlRegister(3, CommonInfoTable.System.Cpu.x64.Cr3); // Restore CR3
      WriteToControlRegister(4, CommonInfoTable.System.Cpu.x64.Cr4); // Restore CR4

    }

    // If gBS isn't NULL (meaning that it's valid), then try to exit using
    // gBS->Exit() - this lets us provide an exit message

    if (gBS != NULL) {

      char16* ExitReason = ((AppStatus == EfiSuccess) ? u"Exiting Serra." : u"Failed to load Serra :(");
      gBS->Exit(ImageHandle, AppStatus, (uint64)(Strlen(ExitReason)), ExitReason);

    }

    // Otherwise, just return whatever's in AppStatus.

    return AppStatus;

}
