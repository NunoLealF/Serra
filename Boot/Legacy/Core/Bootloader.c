// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void SaveState(), RestoreState()

   Inputs: (none)
   Outputs: (none)

   These two functions are called by Shared/Rm/RmWrapper.c (the real mode
   wrapper), and their function is to save and restore the state of the
   system as needed.

   (In this case, they just disable interrupts, and then enable them /
   reload the Interrupt Descriptor Table; nothing special, really.)

*/

descriptorTable IdtDescriptor;

void SaveState(void) {

  __asm__ __volatile__ ("cli");
  return;

}

void RestoreState(void) {

  __asm__ __volatile__ ("sti");
  LoadIdt(&IdtDescriptor);

  return;

}


/* void Bootloader()

   Inputs: (none, except for InfoTable)
   Outputs: (none)

   This is the main function (and entrypoint) of our third-stage bootloader.
   The second stage bootloader (located in Init/, instead of Core/) jumps
   here after it finishes reading Boot/Bootx32.bin from disk.

   This stage of the bootloader occupies the space after 20000h in memory,
   and its role is to initialize as much as possible, before eventually
   loading and transferring control to the kernel (which is still part
   of the boot manager).

   At this point, the CPU is in 32-bit protected mode, with a stack set up at
   20000h (growing downwards), and a regular GDT spanning the entire 32-bit
   address space on both code and data segments. The IDT has not been
   initialized yet, nor has the A20 line been enabled.

   # The memory layout looks like this:

   -> (0h to 7C00h) Unused, *contains IVT* (31 KiB)
   -> (7C00h to 7E00h) First stage bootloader, contains BPB (0.5 KiB)

   -> (7E00h to 9E00h) Second stage bootloader (8 KiB)
   -> (9E00h to AE00h) Real mode wrapper and table (4 KiB)
   -> (AE00h to B000h) *Info table* (0.5 KiB)

   -> (B000h to 10000h) Unused, *contains IDT* (20 KiB)
   -> (10000h to 20000h) *Stack space* (64 KiB)
   -> (20000h onwards) Third stage bootloader (up to 384 KiB)

   The previous stage of our bootloader left us with something called an
   InfoTable, which is essentially a struct (defined in Shared/InfoTables.h)
   that contains some important information: what drive number we're
   booting off of, the status of the debug flag, etc.

   # After reading from the InfoTable, this function does the following:

   -> (1) Reinitialize the terminal table, based off of the values in
   the InfoTable;

   -> (2) Initialize and enable the IDT (Interrupt Descriptor Table),
  as well as the 8259 PIC (which remaps IRQs);

   -> (3) Enable the A20 line, if it hasn't been already;

   -> (4) Obtain the system's memory map via the E820h BIOS function,
   and refine it into a "usable" memory map;

   -> (5) Obtain CPUID data, and check for PAE and long mode support;

   -> (6) Look around for ACPI and SMBIOS tables, and obtain PCI-
   -BIOS information;

   -> (7) Check for VESA VBE support, and attempt to find the best
   possible graphics mode (with text mode as a fallback);

   -> (8) Read the kernel (located in Boot/Serra/Kernel.elf) from
   the current FAT filesystem, and process ELF headers;

   -> (9) Initialize paging, mapping the kernel to a higher-half,
   and identity-mapping everything else;

   -> (10) Prepare the necessary environment to transfer control
   to the kernel, and prepare info tables, before running the
   kernel (via LongmodeStub()).

*/

void Bootloader(void) {

  // [Read from InfoTable]

  // First, let's check to see if the signature is even valid (if not,
  // just return to whichever function called us):

  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTable_Location);

  if (InfoTable->Signature != 0x65363231) {
    return;
  }

  // If it is, then that means we have a valid InfoTable, so let's set
  // the debug flag, and initialize the terminal table:

  Debug = InfoTable->System_Info.Debug;
  Memcpy(&TerminalTable, &InfoTable->Terminal_Info, sizeof(InfoTable->Terminal_Info));

  Putchar('\n', 0);
  Message(Kernel, "Successfully entered the third-stage bootloader.");



  // (Initialize the IDT, as well as the 8259 PIC)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT and 8259 PIC.");

  // First, let's initialize the IDT descriptor with the size and location
  // of our Interrupt Descriptor Table:

  IdtDescriptor.Size = (2048 - 1);
  IdtDescriptor.Offset = IdtLocation;

  // Second, let's set a mask for the PIC (Programmable Interrupt Controller),
  // and initialize it as well:

  MaskPic(0xFFFF); // Full mask, don't enable anything (set to FFFEh for timer, or FFFDh for a PS/2 keyboard)
  InitPic(0x20, 0x28); // IRQ1 is at 20h-27h, IRQ2 is at 28h-2Fh

  // Finally, let's actually enable the IDT:

  MakeDefaultIdtEntries(&IdtDescriptor, 0x08, 0x0F, 0x00);
  LoadIdt(&IdtDescriptor);

  __asm__ __volatile__ ("sti");
  Message(Ok, "Successfully initialized the IDT.");



  // [Set up the A20 line]

  // This can only safely be done *after* initializing the IDT, because some
  // methods aren't really safe without one present (plus, they enable
  // interrupts with sti)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to enable the A20 line");

  bool A20_EnabledByDefault = false;
  bool A20_EnabledByKbd = false;
  bool A20_EnabledByFast = false;

  if (Check_A20() == true) {

    // If the output of the CheckA20 function is true, then that means that
    // the A20 line has already been enabled (either by the firmware
    // itself, or by the BIOS function method in the bootsector).

    A20_EnabledByDefault = true;
    Message(Ok, "The A20 line has already been enabled.");

  } else {

    // If the output of the CheckA20 function is false, then that means that
    // the A20 line hasn't already been enabled by default.

    // In this case, we want to try out two methods to enable the A20 line.
    // The first involves the 8042 keyboard controller:

    Putchar('\n', 0);
    Message(Kernel, "Attempting to enable the A20 line using the 8042 keyboard method");

    EnableKbd_A20();
    Wait_A20();

    if (Check_A20() == true) {

      A20_EnabledByKbd = true;
      Message(Ok, "The A20 line has successfully been enabled.");

    } else {

      // If the first method didn't work, there's also a second method that
      // works on some systems called 'fast A20'.

      // This *could* crash the system, but we don't have much of a choice,
      // since the A20 line is necessary anyways

      Message(Fail, "The A20 line was not successfully enabled.");

      Putchar('\n', 0);
      Message(Kernel, "Attempting to enable the A20 line using the fast A20 method");

      EnableFast_A20();
      Wait_A20();

      if (Check_A20() == true) {

        A20_EnabledByFast = true;
        Message(Ok, "The A20 line has successfully been enabled.");

      } else {

        // If we reach this point, we've exhausted all of the most common
        // methods for enabling the A20 line, which is essential, so the
        // only thing we can really do is give out an error code:

        Panic("Failed to enable the A20 line.", 0);

      }

    }

  }




  // [Get the 'raw' system memory map, using E820]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to obtain the system's memory map, using E820");

  // First, we need to prepare a couple things, and also check to see if
  // it's even supported (NumMmapEntries != 0).

  // Each (E820) memory map entry is 24 bytes long, and some systems can
  // have a huge amount of entries, so we'll reserve space on the stack;
  // 128 entries should hopefully be enough.

  #define MaxMmapEntries 128

  uint8 MmapBuffer[sizeof(mmapEntry) * MaxMmapEntries];
  mmapEntry* Mmap = (mmapEntry*)MmapBuffer;

  // Now, let's actually go ahead and fill MmapBuffer:

  uint8 NumMmapEntries = 0;
  uint8 NumUsableMmapEntries = 0;

  uint32 MmapContinuation = 0;

  do {

    // Get the memory map entry, and save the 'continuation number'

    MmapContinuation = GetMmapEntry((void*)&Mmap[NumMmapEntries], sizeof(mmapEntry), MmapContinuation);

    // Get the number of *usable* memory map entries (type 1 means free
    // memory) - this will be useful later on, for UsableMmap

    if (Mmap[NumMmapEntries].Type == 1) {
      NumUsableMmapEntries++;
    }

    // Increment the number of entries, and break if we've reached the
    // memory map entry limit (MaxMmapEntries)

    NumMmapEntries++;

    if (NumMmapEntries >= MaxMmapEntries) {
      break;
    }

  } while (MmapContinuation != 0);

  // Check status, and if we didn't find any entries (or otherwise just
  // failed to obtain the system memory map), panic.

  if (NumMmapEntries > 0) {

    Message(Ok, "Successfully obtained the system memory map.");

    if (NumMmapEntries >= MaxMmapEntries) {
      Message(Warning, "Memory map entry limit exceeded; final map may be incomplete.");
    }

  } else {

    Panic("Failed to obtain the system's memory map.", 0);

  }




  // [Process the system memory map into a usable memory map]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to process the system memory map.");

  // The first step here is to just sort each entry according to
  // its base address, like this:

  for (uint16 Threshold = 1; Threshold < NumMmapEntries; Threshold++) {

    for (uint16 Position = Threshold; Position > 0; Position--) {

      if (Mmap[Position].Base < Mmap[Position - 1].Base) {
        Memswap((void*)(int)&Mmap[Position], (void*)(int)&Mmap[Position - 1], sizeof(mmapEntry));
      } else {
        break;
      }

    }

  }

  // (Show system memory map entries)

  Message(Ok, "Successfully sorted system memory map entries.");

  for (uint8 Position = 0; Position < NumMmapEntries; Position++) {

    uint64 Start = Mmap[Position].Base;
    uint64 Size = Mmap[Position].Limit;
    uint64 End = (Start + Size);

    Message(Info, "Found type %d memory area from %x:%xh to %x:%xh (%d KiB)", (Mmap[Position].Type),
            (uint32)(Start >> 32), (uint32)Start, (uint32)(End >> 32), (uint32)End, (uint32)(Size / 1024));

  }

  // Next, we just need to isolate all the usable entries, and deal
  // with overlapping entries.

  uint8 UsableMmapBuffer[sizeof(mmapEntry) * NumUsableMmapEntries];
  mmapEntry* UsableMmap = (mmapEntry*)UsableMmapBuffer;

  uint64 MinStart = 0;
  uint8 UsablePosition = 0;

  for (uint8 Position = 0; Position < NumMmapEntries; Position++) {

    // First, get the start and end position of this entry

    uint64 Start = Mmap[Position].Base;
    uint64 End = (Start + Mmap[Position].Limit);

    // Next, update the minimum start, and skip over any entries that
    // end before it (decrementing NumUsableMmapEntries accordingly).

    if (Start < MinStart) {
      Start = MinStart;
    }

    if (End <= MinStart) {

      if (Mmap[Position].Type == 1) {
        NumUsableMmapEntries--;
      }

      continue;

    }

    // Finally, if the entry doesn't overlap with another entry
    // *and* has the right type, we can add it to UsableMmap.

    if (Mmap[Position].Type == 1) {

      // Update the start variable, if necessary

      if (Start < MinStart) {
        Start = MinStart;
      }

      // Update the end variable; for this, we only need to check
      // for the next entry, as the list of entries is already
      // sorted for us

      if (Position < (NumMmapEntries - 1)) {

        if (End > Mmap[Position + 1].Base) {
          End = Mmap[Position + 1].Base;
        }

      }

      // Finally, add the entry to the usable memory map area,
      // and increment the position

      UsableMmap[UsablePosition].Base = Start;
      UsableMmap[UsablePosition].Limit = (End - Start);
      UsableMmap[UsablePosition].Type = Mmap[Position].Type;
      UsableMmap[UsablePosition].Acpi = Mmap[Position].Acpi;

      UsablePosition++;

    }

    MinStart = End;

  }

  // (Show usable memory map entries)

  Message(Ok, "Successfully processed usable memory map entries.");

  for (uint8 Position = 0; Position < NumUsableMmapEntries; Position++) {

    uint64 Start = UsableMmap[Position].Base;
    uint64 Size = UsableMmap[Position].Limit;
    uint64 End = (Start + Size);

    Message(Info, "Found usable memory area from %x:%xh to %x:%xh (%d KiB)",
            (uint32)(Start >> 32), (uint32)Start, (uint32)(End >> 32), (uint32)End, (uint32)(Size / 1024));

  }




  // [Obtain CPUID data]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get data from CPUID");

  // First, let's see if CPUID is even supported - it's essentially
  // required, but not all CPUs that support it have a volatile ID flag
  // (although most do).

  if (SupportsCpuid() == true) {
    Message(Ok, "CPUID appears to be supported (the ID flag is volatile)");
  } else {
    Message(Warning, "CPUID appears to be unsupported; trying anyway");
  }

  // Next, let's obtain a few tables from CPUID, and query it for the
  // highest supported level as well.

  registerTable Cpuid_Info = GetCpuid(0x00000000, 0);
  registerTable Cpuid_Features = GetCpuid(0x00000001, 0);

  registerTable Cpuid_ExtendedInfo = GetCpuid(0x80000000, 0);
  registerTable Cpuid_ExtendedFeatures = GetCpuid(0x80000001, 0);

  uint32 Cpuid_HighestLevel = Cpuid_Info.Eax;

  Message(Ok, "Successfully obtained CPUID data.");

  // Show some info to the user, including the vendor string (which is
  // supplied in eax=00000000h, ecx=0h)

  char VendorString[16];
  GetVendorString(VendorString, Cpuid_Info);

  Message(Info, "Highest supported (standard) CPUID level is %xh", Cpuid_HighestLevel);
  Message(Info, "CPU vendor ID is \'%s\'", VendorString);

  // We also want to check for PAE (Page Address Extensions), PSE (Page Size
  // Extensions) and long mode support; this ends up being necessary later
  // on, so we need to filter out any CPUs that don't support that.

  bool SupportsPae = false;
  bool SupportsPse = false;
  bool SupportsLongMode = false;

  if (Cpuid_Features.Edx & (1 << 3)) {
    SupportsPse = true;
  } else {
    Panic("Non-4KiB pages appear to be unsupported.", 0);
  }

  if (Cpuid_Features.Edx & (1 << 6)) {
    SupportsPae = true;
  } else {
    Message(Warning, "PAE (Page Address Extension) appears to be unsupported.");
  }

  if (Cpuid_ExtendedFeatures.Edx & (1 << 29)) {
    Message(Info, "64-bit mode appears to be supported");
    SupportsLongMode = true;
  } else {
    Panic("64-bit mode appears to be unsupported.", 0);
  }

  // Also check for PAT support - this isn't essential, but it can help
  // with performance later on.

  bool SupportsPat = false;

  if (Cpuid_Features.Edx & (1 << 16)) {
    Message(Info, "PAT appears to be supported");
    SupportsPat = true;
  }




  // [Check for VESA (VBE 2.0+) support, and obtain data if possible]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get VESA-related data.");

  // Let's start by obtaining the VBE info block - this not only lets us
  // see if VESA VBE is supported at all (by checking the return status),
  // but it also lets us filter out older versions of VBE.

  // (For context: this is necessary because not all systems support VBE,
  // because VBE versions before 2.0 don't have support for linear
  // framebuffers, and because VBE lets us deal with graphics modes.)

  volatile vbeInfoBlock VbeInfo;
  uint32 VbeReturnStatus = GetVbeInfoBlock(&VbeInfo);

  bool SupportsVbe;

  if ((VbeReturnStatus != 0x004F) || (VbeInfo.Version < 0x200)) {

    SupportsVbe = false;
    Message(Warning, "VBE (2.0+) appears to be unsupported.");

  } else {

    SupportsVbe = true;
    Message(Ok, "VBE (2.0+) appears to be supported.");
    Message(Info, "The VBE info block table is located at %xh", &VbeInfo);

  }

  // Next, let's check for EDID - this is another protocol that works
  // alongside VBE, but that lets us query the system for things like
  // the supported display resolution.

  // (VBE tells us which modes/resolutions the *graphics card* supports,
  // but we still don't know which ones the monitor supports; that's
  // where EDID comes in.)

  volatile edidInfoBlock EdidInfo;

  uint32 EdidReturnStatus = GetEdidInfoBlock(&EdidInfo, 0x00);
  bool SupportsEdid = false;

  if (SupportsVbe == true) {

    if (((EdidReturnStatus & 0xFF) != 0x4F) || (SupportsVbe == false)) {

      SupportsEdid = false;
      Message(Warning, "EDID appears to be unsupported; using lowest video mode.");

    } else {

      SupportsEdid = true;
      Message(Ok, "EDID appears to be supported.");
      Message(Info, "The EDID info block table is located at %xh", &EdidInfo);

    }

  }

  // Based off of what EDID tells us, we can try to find the preferred
  // resolution - either it's supported (in which case, we need to look
  // at the timings), or it isn't (which means we can use 720*480).

  edidDetailedTiming PreferredTimings = EdidInfo.DetailedTimings[0];
  uint16 PreferredResolution[2] = {720, 480};

  if (SupportsEdid == true) {

    PreferredResolution[0] = (PreferredTimings.Timings.HorizontalInfo_Low & 0xFF) | (PreferredTimings.Timings.HorizontalInfo_High & 0xF0) << 4;
    PreferredResolution[1] = (PreferredTimings.Timings.VerticalInfo_Low & 0xFF) | (PreferredTimings.Timings.VerticalInfo_High & 0xF0) << 4;

    Message(Info, "Preferred resolution appears to be %d*%d (according to EDID)",
           (uint32)PreferredResolution[0], (uint32)PreferredResolution[1]);

  }

  // Okay - now that we have all of that information, we can finally
  // go ahead and try to find the best graphics mode for this system.

  // (If VBE isn't supported, we can still use text mode as a fallback)

  uint16 BestVbeMode = 0xFFFF;
  volatile vbeModeInfoBlock BestVbeModeInfo;

  if (SupportsVbe == true) {

    BestVbeMode = FindBestVbeMode((uint16*)(convertFarPtr(VbeInfo.VideoModeListPtr)), PreferredResolution[0], PreferredResolution[1]);
    GetVbeModeInfo(&BestVbeModeInfo, BestVbeMode);

    Message(Info, "Using mode %xh, with a %d*%d resolution and a %d-bit color depth", (uint32)BestVbeMode,
            BestVbeModeInfo.ModeInfo.X_Resolution, BestVbeModeInfo.ModeInfo.Y_Resolution, BestVbeModeInfo.ModeInfo.BitsPerPixel);

  } else {

    Message(Info, "Using text mode as a fallback (due to the lack of VBE support)");

  }




  // [Obtain ACPI and SMBIOS tables]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to obtain ACPI and SMBIOS tables");

  // First, let's try to find the main ACPI tables (the RSDP and RSDT/XSDT):

  acpiRsdpTable* Rsdp = GetAcpiRsdpTable();

  bool SupportsAcpi;

  if (Rsdp == NULL) {

    SupportsAcpi = false;
    Message(Warning, "ACPI appears to be unsupported (unable to find table).");

  } else {

    SupportsAcpi = true;
    Message(Ok, "Successfully located ACPI tables.");

    Message(Info, "ACPI RSDP table is located at %xh", (uint32)Rsdp);
    Message(Info, "RSDT seems to be located at %xh, XSDT may be located at %x:%xh", (uint32)(Rsdp->Rsdt), (uint32)((Rsdp->Xsdt) >> 32), (uint32)(Rsdp->Xsdt));

  }

  // Next, let's do the same, but for the SMBIOS tables:

  void* SmbiosEntryPoint = GetSmbiosEntryPointTable();
  bool SupportsSmbios;

  if (SmbiosEntryPoint == NULL) {

    SupportsSmbios = false;
    Message(Warning, "SMBIOS appears to be unsupported (unable to find table).");

  } else {

    SupportsSmbios = true;
    Message(Ok, "Successfully located SMBIOS tables.");
    Message(Info, "SMBIOS entry point table appears to be located at %xh", (uint32)SmbiosEntryPoint);

  }




  // [Obtain PCI-BIOS data]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get PCI-BIOS data");

  // We only need to call GetPciBiosInfoTable(), which in turn calls
  // the BIOS function (int 1Ah, ax B101h).

  pciBiosInfoTable PciBiosTable;
  uint32 PciBiosReturnStatus = GetPciBiosInfoTable(&PciBiosTable);

  bool SupportsPciBios;

  if ((PciBiosReturnStatus & 0xFF00) != 0) {

    SupportsPciBios = false;
    Message(Fail, "Failed to obtain information using the PCI-BIOS interrupt call.");
    Message(Warning, "PCI BIOS may not be available on this system.");

  } else {

    SupportsPciBios = true;
    Message(Info, "Successfully obtained information using the PCI-BIOS interrupt call.");

  }




  // [Preparing disk and (FAT) filesystem drivers]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get EDD/FAT data");

  // First, let's read from the InfoTable to collect some information about
  // the current system - the (int 13h) drive number, the filesystem type,
  // the physical and logical sector size, etc.

  DriveNumber = InfoTable->DriveNumber;
  bool Edd_Enabled = InfoTable->Edd_Enabled;

  LogicalSectorSize = InfoTable->LogicalSectorSize;
  PhysicalSectorSize = InfoTable->PhysicalSectorSize;

  bool IsFat32 = InfoTable->Bpb_IsFat32;

  Message(Info, "Successfully obtained drive/EDD-related information.");

  // Next, let's find the BPB (BIOS Parameter Block); this will tell us
  // more information about the filesystem.

  // (For reference: FAT filesystems have a structure in the bootsector
  // called a BIOS Parameter Block, and which is defined differently
  // based on whether the filesystem is FAT16 or FAT32)

  #define Bpb_Address (&InfoTable->Bpb[0])

  biosParameterBlock Bpb = *(biosParameterBlock*)(Bpb_Address);

  biosParameterBlock_Fat16 Extended_Bpb16 = *(biosParameterBlock_Fat16*)(Bpb_Address + 33);
  biosParameterBlock_Fat32 Extended_Bpb32 = *(biosParameterBlock_Fat32*)(Bpb_Address + 33);

  // (Just in case, let's do a sanity check)

  if (Bpb.BytesPerSector < 512) {
    Panic("Failed to obtain the BPB.", 0);
  } else {
    Message(Info, "Successfully obtained the BPB from the bootloader's info table.");
  }

  // Finally, now that we know where it is, we can start reading from the
  // BPB. We'll collect some important information, such as:

  // -> The total number of sectors within the partition (incl. reserved)

  uint32 TotalNumSectors = Bpb.NumSectors;

  if (TotalNumSectors == 0) {
    TotalNumSectors = Bpb.NumSectors_Large;
  }

  // -> The size of each FAT (File Allocation Table), in sectors

  uint32 FatSize = Bpb.SectorsPerFat;

  if (FatSize == 0) {
    FatSize = Extended_Bpb32.SectorsPerFat;
  }

  // -> The number of root sectors in the partition

  uint32 NumRootSectors = ((Bpb.NumRootEntries * 32) + (LogicalSectorSize - 1)) / LogicalSectorSize;

  // -> The offset of the first data sector within the partition

  uint32 DataSectorOffset = ((Bpb.NumFileAllocationTables * FatSize) + NumRootSectors) + Bpb.ReservedSectors;

  // -> The number of data sectors within the partition

  uint32 NumDataSectors = (TotalNumSectors - DataSectorOffset);

  // -> The cluster limit (the last cluster that can be considered valid)

  uint32 ClusterLimit = 0xFFF6;

  if (IsFat32 == true) {
    ClusterLimit = 0x0FFFFFF6;
  }

  // -> The root cluster offset, and the root sector offset (this is where
  // we should start looking for data from)

  uint32 RootCluster;

  if (IsFat32 == false) {
    RootCluster = 2;
  } else {
    RootCluster = Extended_Bpb32.RootCluster;
  }

  uint32 RootSectorOffset = DataSectorOffset;

  if (IsFat32 == false) {
    RootSectorOffset -= NumRootSectors;
  }

  // (Sanity check our values, and show progress)

  if ((NumDataSectors == 0) || (RootSectorOffset == 0)) {

    Panic("Failed to read data from the BPB.", 0);

  } else {

    Message(Ok, "Successfully read data from the BPB.");
    Message(Info, "FAT%d Filesystem has %d sectors (%d root, %d data)", ((IsFat32 == true) ? 32: 16), TotalNumSectors, NumRootSectors, NumDataSectors);
    Message(Info, "Root cluster offset is %d; cluster limit is %xh", RootCluster, ClusterLimit);

  }

  // Now that we've set that up, we can finally start looking for the kernel
  // directory (Boot/Serra/Kernel.elf); we won't be loading it just yet, but
  // we do need to know where it is.

  // (Start by searching for Boot/ within the root directory)

  fatDirectory BootDirectory = FindDirectory(RootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, RootSectorOffset, "BOOT    ", "   ", true, IsFat32);
  uint32 BootCluster = GetDirectoryCluster(BootDirectory);

  if (ExceedsLimit(BootCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/.", 0);
  }

  // (Then, search for Serra/ within Boot/)

  fatDirectory SerraDirectory = FindDirectory(BootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "SERRA   ", "   ", true, IsFat32);
  uint32 SerraCluster = GetDirectoryCluster(SerraDirectory);

  if (ExceedsLimit(SerraCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/Serra/.", 0);
  }

  // (Finally, look for Kernel.elf within Boot/Serra/)

  fatDirectory KernelDirectory = FindDirectory(SerraCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "KERNEL  ", "ELF", false, IsFat32);
  uint32 KernelCluster = GetDirectoryCluster(KernelDirectory);

  if (ExceedsLimit(KernelCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/Serra/Kernel.elf.", 0);
  } else {
    Message(Ok, "Successfully located Boot/Serra/Kernel.elf.");
  }



  // [Allocate space for the kernel and any initial page tables]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to allocate space for the kernel and initial page tables.");

  // First, we need to find a suitable starting point. The stack isn't large
  // enough for our page tables, so we need to find a new way to allocate
  // memory, which in this case, is AllocateFromMmap().

  // That function essentially works as a bump allocator, which means that
  // it only 'allocates' by returning the *next* contiguous block of memory
  // (within the usable memory map) that fits the criteria.

  // That means that we need a starting point - which we call 'Offset' - in
  // order to actually use the function; in this case, we're just looking
  // for the earliest possible point after 1 MiB (100000h).

  uint64 Offset = 0x100000;

  for (uint8 Entry = 0; Entry < NumUsableMmapEntries; Entry++) {

    if (UsableMmap[Entry].Base >= 0x100000) {

      Offset = UsableMmap[Entry].Base;
      break;

    }

  }

  // Now that we've defined Offset, we can start allocating space from our
  // usable memory map. We'll start off by allocating space for the kernel
  // executable:

  uintptr KernelPtr = (uintptr)(AllocateFromMmap(Offset, PageAlign(KernelDirectory.Size), false, UsableMmap, NumUsableMmapEntries));

  if (KernelPtr == 0) {

    Panic("Unable to allocate enough space for the kernel.", 0);

  } else {

    Offset = KernelPtr;
    KernelPtr -= PageAlign(KernelDirectory.Size);

    Message(Ok, "Allocated kernel space between %xh and %xh (in pmem).", KernelPtr, (uint32)Offset);

  }

  // Next, let's also allocate some space for the kernel stack:

  #define KernelStackSize 0x100000 // Must be a multiple of 4KiB
  uintptr KernelStack = (uintptr)(AllocateFromMmap(Offset, KernelStackSize, false, UsableMmap, NumUsableMmapEntries));

  if (KernelStack == 0) {

    Panic("Unable to allocate enough space for the kernel stack.", 0);

  } else {

    Offset = KernelStack;
    KernelStack -= KernelStackSize;

    Message(Ok, "Allocated kernel stack space between %xh and %xh (in pmem).", KernelStack, (uint32)(Offset));

  }

  // Finally, let's allocate space for the PML4 tables; this will be useful
  // when we enable paging later on.

  // (Keep in mind that AllocateFromMmap() *always* returns an offset on
  // a page boundary (meaning that our PML4 is page-aligned), and that in
  // this case, it's also zeroed out.)

  uintptr Pml4 = (uintptr)(AllocateFromMmap(Offset, (512 * 8), true, UsableMmap, NumUsableMmapEntries));
  uint64* Pml4_Data;

  if (Pml4 == 0) {

    Panic("Unable to allocate enough space for the page tables.", 0);

  } else {

    Offset = Pml4;
    Pml4 -= (512 * 8); Pml4_Data = (uint64*)Pml4;

    Message(Ok, "Allocated PML4 space between %xh and %xh (in pmem).", Pml4, (uint32)(Offset));

  }




  // [Identity-mapping 'low' (below IdentityMapThreshold) and usable memory]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to identity map low and usable memory.");

  #define UsableFlags (pagePresent | pageRw)
  #define IdmappedFlags (UsableFlags | pagePcd) // (Add pageSize for 2MiB pages)

  // In order to enable long mode - and eventually load our kernel - we
  // first need to enable paging, which is a CPU feature that introduces
  // the virtual address space.

  // In that environment, you can essentially map any physical memory
  // location to any virtual memory location - for example, our kernel is
  // always loaded at FFFFFFFF80000000h, no matter where it resides
  // within physical memory.

  // However, this also introduces a problem: not everything in memory can
  // tolerate suddenly changing location. In fact, when we enable paging,
  // the CPU *immediately* switches to virtual addressing, without even
  // changing the instruction pointer.

  // Because of that, we need to map *some* pages to the same virtual
  // address as their physical address - this is called *identity mapping*,
  // and it's necessary for anything that can't change address.

  // For now, we'll start by identity mapping everything up to
  // IdentityMapThreshold (1000000h, or 16 MiB):

  #define IdentityMapThreshold 0x1000000 // (16 MiB; must be a multiple of 2 MiB)

  Offset = InitializePageEntries(0, 0, IdentityMapThreshold, Pml4_Data, IdmappedFlags, true, false, Offset, UsableMmap, NumUsableMmapEntries);
  Message(Ok, "Successfully identity mapped the first %d MiB of memory", (IdentityMapThreshold / 0x100000));

  // Additionally, we'll also identity map everything in the usable memory
  // map - this is so the kernel can use it later, and to avoid any crashes
  // if we do allocate something above IdentityMapThreshold.

  for (uint16 Entry = 0; Entry < NumUsableMmapEntries; Entry++) {

    // Get variables

    uint64 Start = UsableMmap[Entry].Base;
    uint64 Size = UsableMmap[Entry].Limit;

    // Align to 2MiB (huge page) boundaries; more specifically, align
    // Start *down*, but align Size *up*

    Start -= (Start % 0x200000);

    if ((Size % 0x200000) != 0) {
      Size += (0x200000 - (Size % 0x200000));
    }

    // Initialize page entries

    Offset = InitializePageEntries(Start, Start, Size, Pml4_Data, IdmappedFlags, true, false, Offset, UsableMmap, NumUsableMmapEntries);
    Message(Ok, "Successfully identity mapped %d MiB area starting at %x:%xh", (uint32)(Size / 0x100000), (uint32)(Start >> 32), (uint32)Start);

  }

  // Also, if PAT is supported, add support for write-combining by updating
  // its MSR (the default value is usually 0007040600070406h, where PAT4
  // is 00h (uncached) instead of 01h (write-combining)).

  if (SupportsPat == true) {

    #define PatMsrValue 0x0007040601070406

    WriteToMsr(patMsr, PatMsrValue);
    Message(Ok, "Updated the PAT MSR to %x%xh.", (uint32)(PatMsrValue >> 32), (uint32)PatMsrValue);

  }

  // Finally, if VBE is supported, then we also have to map the
  // framebuffer, since it generally isn't located within usable memory.

  // (If PAT is supported, we also map it as *write-combining*; in theory,
  // this should help improve performance, since it essentially works as
  // a form of double buffering.)

  if (SupportsVbe == true) {

    // Get address and size

    uint32 Address = BestVbeModeInfo.Vbe2Info.Framebuffer;
    uint32 Size = BestVbeModeInfo.ModeInfo.Y_Resolution;

    if (VbeInfo.Version >= 0x300) {
      Size *= BestVbeModeInfo.Vbe3Info.LinearBytesPerScanLine;
    } else {
      Size *= BestVbeModeInfo.ModeInfo.BytesPerScanLine;
    }

    // Align to 2 MiB boundaries, the same way we did for the usable mmap

    Address -= (Address % 0x200000);

    if ((Size % 0x200000) != 0) {
      Size += (0x200000 - (Size % 0x200000));
    }

    // Initialize page entries. If PAT is enabled, we enable the PAT bit,
    // so that the framebuffer can be mapped as write-combining.

    Offset = InitializePageEntries(Address, Address, Size, Pml4_Data, UsableFlags, true, true, Offset, UsableMmap, NumUsableMmapEntries);
    Message(Ok, "Successfully identity mapped %d MiB framebuffer starting at 0:%xh", (Size / 0x100000), Address);

  }



  // [Loading and running the kernel]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to load and run the kernel.");

  // First, let's read the kernel file from disk. We already obtained the
  // directory earlier (KernelDirectory), and allocated space for it in
  // memory (KernelPtr), so all that's left is to call ReadFile().

  bool ReadFileSuccessful = ReadFile((void*)KernelPtr, KernelDirectory, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, IsFat32);

  if (ReadFileSuccessful == true) {
    Message(Ok, "Successfully loaded Boot/Serra/Kernel.elf to %xh.", (uint32)KernelPtr);
  } else {
    Panic("Failed to read Boot/Serra/Kernel.elf from disk.", 0);
  }






  // *TODO*: Comment this properly.
  // (Also, note for future self - just *don't* use anything
  // compression-related, it's somehow slower than int 13h lmao)






  // [3.2] Actually read the ELF file
  // (TODO: implement a proper ELF driver, lol.)


  // [3.2.1] Check to see if it's a valid ELF file, and show some
  // basic debug information.

  elfHeader* KernelHeader = (elfHeader*)KernelPtr;

  if (KernelHeader->Ident.MagicNumber != 0x464C457F) {
    Panic("Kernel does not appear to be an actual ELF file.", 0);
  } else if ((KernelHeader->Ident.Class != 2) || (KernelHeader->MachineType != 0x3E)) {
    Panic("Kernel does not appear to be 64-bit.", 0);
  } else if ((KernelHeader->ProgramHeaderOffset == 0) || (KernelHeader->NumProgramHeaders == 0)) {
    Panic("ELF header does not appear to have any program headers.", 0);
  } else if (KernelHeader->Version < 1) {
    Message(Warning, "ELF header version appears to be invalid (%d)", (uint32)KernelHeader->Version);
  } else if (KernelHeader->Entrypoint == 0) {
    Message(Warning, "ELF header does not have an entrypoint");
  } else if (KernelHeader->FileType != 2) {
    Message(Warning, "ELF header appears to have a non-executable file type (%d)", (uint32)KernelHeader->FileType);
  } else {
    Message(Ok, "Kernel appears to be a valid ELF executable.");
  }

  Message(Info, "File type: %d, machine type: %xh, version: %d, entry: %x:%xh",
                (uint32)KernelHeader->FileType, (uint32)KernelHeader->MachineType, (uint32)KernelHeader->Version,
                (uint32)(KernelHeader->Entrypoint >> 32), (uint32)(KernelHeader->Entrypoint & 0xFFFFFFFF));

  Message(Info, "Real physical address (start of file) at %xh", (uint32)KernelPtr);

  Message(Info, "%d program header(s) at +%xh, %d section header(s) at +%xh",
                (uint32)(KernelHeader->NumProgramHeaders), (uint32)(KernelHeader->ProgramHeaderOffset & 0xFFFFFFFF),
                (uint32)(KernelHeader->NumSectionHeaders), (uint32)(KernelHeader->SectionHeaderOffset & 0xFFFFFFFF));




  // [3.2.2] Read program headers.. may be best to put this off until necessary though
  // (like, until we have a function that automatically maps this stuff, better not to)

  Putchar('\n', 0);
  Message(-1, "(TODO) Read program headers");

  for (uint32 Index = 0; Index < KernelHeader->NumProgramHeaders; Index++) {

    elfProgramHeader* Header = GetProgramHeader(KernelPtr, KernelHeader, Index);

    Message(Info, "ProgramHeader(%d) | Type %d | Flags %xh | Offset %xh", Index, (uint32)Header->Type, (uint32)Header->Flags, (uint32)Header->Offset);
    Message(Info, "ProgramHeader(%d) | Address %x:%xh, Size %d and %d bytes", Index, (uint32)(Header->VirtAddress >> 32), (uint32)Header->VirtAddress, (uint32)Header->Size, (uint32)Header->PaddedSize);

    if (Header->Type == 0x01) {

      uint64 PhysAddress = (uint64)(KernelPtr) + Header->Offset;
      Offset = InitializePageEntries(PhysAddress, Header->VirtAddress, Header->Size, Pml4_Data, UsableFlags, false, false, Offset, UsableMmap, NumUsableMmapEntries);

    }

  }

  // Todo: there seems to be some alignment issues? but let me see if it works okay

  Message(Kernel, "Mapping kernel stack; offset = %xh", (uint32)Offset);
  Offset = InitializePageEntries((uint64)KernelStack, (KernelHeader->Entrypoint - KernelStackSize), KernelStackSize, Pml4_Data, UsableFlags, false, false, Offset, UsableMmap, NumUsableMmapEntries);
  Message(Ok, "Okay done; offset = %xh", (uint32)Offset);

  // [3.3 / 4.1 / idk] Remap as necessary
  // (Kernel will be in the last ([511], 512th) PML4, at FFFFFF.FF80000000-FFFFFF.FFFFFFFFFFh) -> KernelPtr
  // (Kernel stack will be in the second-to-last ([510], 511th) PML4, at FFFFFF.FF00000000-FFFFFF.FF7FFFFFFFh) -> KernelStack

  Putchar('\n', 0);
  Message(Kernel, "Preparing to transfer control to the kernel.");

  Message(-1, "(TODO) Remap kernel+stack as necessary");









  // [4.2] Prepare info tables, set resolution, etc. etc.

  Message(-1, "(TODO) Set up infotables, resolution, etc.");

  // [4.3] Call Lmstub

  Message(-1, "(TODO) Call LongmodeStub()");



  // [For now, let's just leave things here]

  Debug = true;


  char* Teststring = "This is also Serra! <3                                                          "
                     "(long mode kernel edition)                                                      ";


  Putchar('\n', 0);

  Printf("Hi, this is Serra! <3\n", 0x0F);
  Printf("April %i %x\n", 0x3F, 18, 0x2025);

  // When the time comes...
  //if (SupportsVbe == true) SetVbeMode(BestVbeMode, false, true, true, NULL);
  LongmodeStub((uintptr)Teststring, Pml4);

  for(;;);

}
