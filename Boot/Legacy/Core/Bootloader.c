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

   These two functions are called by Shared/Rm/RmWrapper.c, and they basically just save and
   restore the state of the system (for example, they might restore the IDT, paging, etc.)

   (TODO: actually comment this better, the specifics of what this is guaranteed to vary
   quite a bit)

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


/* void __attribute__((noreturn)) Bootloader()

   Inputs:    (none)
   Outputs:   (none)

   This is our third-stage bootloader's main function. We jump here after Init().

   TODO: Write a more concise description. (I feel like this is only really possible after actually
   finishing this part, lol)

*/

// 7C00h -> 7E00h: first stage bootloader, contains BPB, 0.5 KiB
// 7E00h -> 9E00h: second stage bootloader, 8 KiB
// 9E00h -> AC00h: real mode
// AC00h -> AE00h: real mode table
// AE00h -> B000h: info table
// B000h -> B800h: interrupt descriptor table
// B800h -> 10000h: (empty)
// 10000h -> 20000h: stack, 64 KiB
// 20000h -> 80000h: third stage bootloader, 384 KiB

void Bootloader(void) {

  // (Get info table - check if signature is correct)

  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTable_Location);

  if (InfoTable->Signature != 0x65363231) {
    return;
  }

  // (Get info table, update debug)

  Debug = InfoTable->System_Info.Debug;



  // (Initialize terminal table..)

  Memcpy(&TerminalTable, &InfoTable->Terminal_Info, sizeof(InfoTable->Terminal_Info));

  Putchar('\n', 0);
  Message(Kernel, "Successfully entered the third-stage bootloader.");



  // (Prepare to initialize IDT and PIC)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to initialize the IDT.");

  IdtDescriptor.Size = (2048 - 1);
  IdtDescriptor.Offset = IdtLocation;

  // (PIC section)

  Message(Kernel, "Initializing the 8259 PIC.");

  MaskPic(0xFFFF); // Full mask, don't enable anything (set to 0xFFFE for timer, or 0xFFFD for keyboard that doesn't really work)
  InitPic(0x20, 0x28); // IRQ1 is at 0x20-0x27, IRQ2 is at 0x28-0x2F

  // (Actually initialize the IDT)

  MakeDefaultIdtEntries(&IdtDescriptor, 0x08, 0x0F, 0x00);
  LoadIdt(&IdtDescriptor);

  __asm__("sti");
  Message(Ok, "Successfully initialized the IDT and PIC.");




  // (Set up A20)

  // This can only safely be done *after* initializing the IDT, because some methods aren't
  // really safe without one present (and also, they enable interrupts with sti)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to enable the A20 line");

  bool A20_EnabledByDefault = false;
  bool A20_EnabledByKbd = false;
  bool A20_EnabledByFast = false;

  if (Check_A20() == true) {

    // If the output of the CheckA20 function is true, then that means that the A20 line has
    // already been enabled.

    A20_EnabledByDefault = true;
    Message(Ok, "The A20 line has already been enabled.");

  } else {

    // If the output of the CheckA20 function is false, then that means that the A20 line has not
    // already been enabled.

    // In this case, we want to try out two methods to enable the A20 line, the first of which
    // involves the 8042 keyboard controller.

    Putchar('\n', 0);
    Message(Kernel, "Attempting to enable the A20 line using the 8042 keyboard method.");

    EnableKbd_A20();
    Wait_A20();

    if (Check_A20() == true) {

      A20_EnabledByKbd = true;
      Message(Ok, "The A20 line has successfully been enabled.");

    } else {

      // If the first method didn't work, there's also a second method that works on some systems
      // called 'fast A20'.
      // This may crash the system, but we'll have to reset if we can't enable A20 anyways.

      Message(Fail, "The A20 line was not successfully enabled.");

      Putchar('\n', 0);
      Message(Kernel, "Attempting to enable the A20 line using the fast A20 method.");

      EnableFast_A20();
      Wait_A20();

      if (Check_A20() == true) {

        A20_EnabledByFast = true;
        Message(Ok, "The A20 line has successfully been enabled.");

      } else {

        // At this point, we've exhausted all of the most common methods for enabling A20 (such
        // as the aforementioned BIOS interrupt, the 8042 keyboard controller method, and the
        // fast A20 method.

        // As it's necessary for us to enable the A20 line, we'll need to crash the system /
        // give out an error code if we get to this point.

        Panic("Failed to enable the A20 line.", 0);

      }

    }

  }



  // [Get the *raw* E820 memory map]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to obtain the system's memory map, using E820");

  // First, we need to prepare, and check if it even works; each entry is 24 bytes, and
  // some systems can have a huge amount of entries, so we'll reserve space on the stack
  // (128 entries should be enough)

  #define MaxMmapEntries 128

  uint8 MmapBuffer[sizeof(mmapEntry) * MaxMmapEntries];
  mmapEntry* Mmap = (mmapEntry*)MmapBuffer;

  // Now, let's actually go ahead and fill that

  uint8 NumMmapEntries = 0;
  uint8 NumUsableMmapEntries = 0;

  uint32 MmapContinuation = 0;

  do {

    // Get the memory map entry, and save the continuation number in MmapContinuation

    MmapContinuation = GetMmapEntry((void*)&Mmap[NumMmapEntries], sizeof(mmapEntry), MmapContinuation);

    // Get the number of *usable* memory map entries

    if (Mmap[NumMmapEntries].Type == 1) {
      NumUsableMmapEntries++;
    }

    // Increment the number of entries, and break if we've reached the mmap entry limit

    NumMmapEntries++;

    if (NumMmapEntries >= MaxMmapEntries) {
      break;
    }

  } while (MmapContinuation != 0);


  // Show messages, if necessary

  if (NumMmapEntries > 0) {

    Message(Ok, "Successfully obtained the system memory map.");

    if (NumMmapEntries >= MaxMmapEntries) {
      Message(Warning, "Memory map entry limit exceeded; final map may be incomplete.");
    }

  } else {

    Panic("Failed to obtain the system's memory map.", 0);

  }




  // [Process the E820 memory map]

  // The first step here is to just sort each entry according to its base address,
  // like this (this is an O(n²) algorithm):

  for (uint16 Threshold = 1; Threshold < NumMmapEntries; Threshold++) {

    for (uint16 Position = Threshold; Position > 0; Position--) {

      if (Mmap[Position].Base < Mmap[Position - 1].Base) {
        Memswap((void*)(int)&Mmap[Position], (void*)(int)&Mmap[Position - 1], sizeof(mmapEntry));
      } else {
        break;
      }

    }

  }

  // Next, we just need to isolate all the usable entries, and deal with overlapping
  // entries. (might be a good idea to put this in a function..)

  uint8 UsableMmapBuffer[sizeof(mmapEntry) * NumUsableMmapEntries];
  mmapEntry* UsableMmap = (mmapEntry*)UsableMmapBuffer;

  uint64 MinStart = 0;
  uint8 UsablePosition = 0;

  for (uint8 Position = 0; Position < NumMmapEntries; Position++) {

    // Get the start and end position of this entry

    uint64 Start = Mmap[Position].Base;
    uint64 End = (Start + Mmap[Position].Limit);

    // Next, update the minimum start, and skip over any entries that end before it
    // (if it's a usable/'free' entry, then decrement the number of usable/'free' entries)

    if (Start < MinStart) {
      Start = MinStart;
    }

    if (End <= MinStart) {

      if (Mmap[Position].Type == 1) {
        NumUsableMmapEntries--;
      }

      continue;

    }

    // Finally, if the type is right, then let's go ahead and actually add the entry
    // (if the type shows that it's usable)

    if (Mmap[Position].Type == 1) {

      // Update the start variable, if necessary

      if (Start < MinStart) {
        Start = MinStart;
      }

      // Update the end variable; for this, we only need to check for the next entry,
      // as the list of entries is already sorted for us

      if (Position < (NumMmapEntries - 1)) {

        if (End > Mmap[Position + 1].Base) {
          End = Mmap[Position + 1].Base;
        }

      }

      // Finally, add the entry to the usable memory map area, and increment the position

      UsableMmap[UsablePosition].Base = Start;
      UsableMmap[UsablePosition].Limit = (End - Start);
      UsableMmap[UsablePosition].Type = Mmap[Position].Type;
      UsableMmap[UsablePosition].Acpi = Mmap[Position].Acpi;

      UsablePosition++;

    }

    // Update MinStart

    MinStart = End;

  }

  // (Everything is done; we have the number of usable mmap entries in NumUsableMmapEntries)

  Message(Ok, "Successfully sorted the system memory map entries.");

  for (uint8 Position = 0; Position < NumUsableMmapEntries; Position++) {

    // TODO: This doesn't accurately show 64-bit addresses (above 4GiB), or sizes above 4
    // TiB, due to the limits of Itoa() (since our implementation only accepts 32-bit numbers)

    uint32 Base = (uint32)(UsableMmap[Position].Base & 0xFFFFFFFF);
    uint32 Limit = (uint32)((UsableMmap[Position].Base + UsableMmap[Position].Limit) & 0xFFFFFFFF);

    uint32 Size = (uint32)(UsableMmap[Position].Limit / 1024);

    Message(Info, "Free memory area from %xh to %xh (%d KiB)", Base, Limit, Size);

  }




  // [Get CPUID data]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get data from CPUID");

  // (First, let's see if it's even supported)

  if (SupportsCpuid() == true) {
    Message(Ok, "CPUID appears to be supported (the ID flag is volatile)");
  } else {
    Message(Warning, "CPUID appears to be unsupported; trying anyway");
  }

  // (Next, let's get some important data from CPUID)

  registerTable Cpuid_Info = GetCpuid(0x00000000, 0);
  registerTable Cpuid_Features = GetCpuid(0x00000001, 0);

  registerTable Cpuid_ExtendedInfo = GetCpuid(0x80000000, 0);
  registerTable Cpuid_ExtendedFeatures = GetCpuid(0x80000001, 0);

  uint32 Cpuid_HighestLevel = Cpuid_Info.Eax;

  Message(Ok, "Successfully obtained CPUID data.");

  // (Show info; for now we're just collecting data, but it would definitely help to
  // make this more complete tbh)

  char VendorString[16];
  GetVendorString(VendorString, Cpuid_Info);

  Message(Info, "Highest supported (standard) CPUID level is %xh", Cpuid_HighestLevel);
  Message(Info, "CPU vendor ID is \'%s\'", VendorString);

  // (This is also important - check for PAE, PSE and long-mode support, so we know how to
  // implement paging)

  bool SupportsPae = false;
  bool SupportsPse = false;
  bool SupportsLongMode = false;

  if (Cpuid_Features.Edx & (1 << 3)) {
    SupportsPse = true;
  } else {
    Panic("Non-4KiB pages appear to be unsupported.", 0);
  }

  if (Cpuid_Features.Edx & (1 << 6)) {
    Message(Info, "PAE (Page Address Extension) appears to be supported");
    SupportsPae = true;
  }

  if (Cpuid_ExtendedFeatures.Edx & (1 << 29)) {
    Message(Info, "64-bit mode appears to be supported");
    SupportsLongMode = true;
  }




  // (...)

  // Okay, so, for reference - a memory manager isn't really necessary at this stage

  // (as for PCI, even if you ignore how it doesn't really belong, it's pretty complicated,
  // and the documentation on it is garbage)

  // The most I might do is implement (int 1Ah, ax B101h), since it might be useful for a few
  // systems



  // [ACPI]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get ACPI info");

  // I think all that's actually needed for now is to just get the RSDP/XSDP, but other
  // than that, I'm pretty sure nothing else is needed

  // That still takes some work though

  // (get table and check for ACPI support)

  acpiRsdpTable* AcpiRsdp = GetAcpiRsdpTable();
  bool AcpiIsSupported = true;

  if (AcpiRsdp == NULL) {
    AcpiIsSupported = false;
  }

  // (show info)

  if (AcpiIsSupported == true) {

    Message(Ok, "RSDP exists at %xh", (uint32)AcpiRsdp);
    Message(Info, "ACPI revision is %d, RSDT exists at %xh, XSDT may exist at %x:%xh", AcpiRsdp->Revision, AcpiRsdp->Rsdt, (uint32)(AcpiRsdp->Xsdt >> 32), (uint32)(AcpiRsdp->Xsdt & 0xFFFFFFFF));

  } else {

    Message(Warning, "RSDP doesn't exist?");

  }




  // [VESA/VBE, and maybe EDID]
  // https://pdos.csail.mit.edu/6.828/2012/readings/hardware/vbe3.pdf (important)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get VESA-related data.");

  // (call it)

  volatile vbeInfoBlock VbeInfo;
  uint32 VbeReturnStatus = GetVbeInfoBlock(&VbeInfo);

  // (check to see if it's supported)

  bool VbeIsSupported = true;

  if (VbeReturnStatus != 0x004F) {
    VbeIsSupported = false;
  }

  // (show info on whether VBE is supported)

  if (VbeIsSupported == true) {
    Message(Ok, "VBE appears to be supported (the table itself is at %xh).", &VbeInfo);
  } else {
    Message(Warning, "VBE appears to be unsupported.");
  }

  // (check for EDID)

  volatile edidInfoBlock EdidInfo;

  bool EdidIsSupported = true;
  uint32 EdidReturnStatus = GetEdidInfoBlock(&EdidInfo, 0x00);

  if (((EdidReturnStatus & 0xFF) != 0x4F) || (VbeIsSupported == false)) {
    EdidIsSupported = false;
  }

  // (show info on whether EDID is supported)

  if (EdidIsSupported == true) {
    Message(Ok, "EDID appears to be supported (the table itself is at %xh).", &EdidInfo);
  } else {
    Message(Warning, "EDID appears to be unsupported; using lowest video mode.");
  }

  // (get preferred resolution)

  edidDetailedTiming PreferredTimings = EdidInfo.DetailedTimings[0];
  uint16 PreferredResolution[2];

  if (EdidIsSupported == true) {

    PreferredResolution[0] = (PreferredTimings.Timings.HorizontalInfo_Low & 0xFF) | (PreferredTimings.Timings.HorizontalInfo_High & 0xF0) << 4;
    PreferredResolution[1] = (PreferredTimings.Timings.VerticalInfo_Low & 0xFF) | (PreferredTimings.Timings.VerticalInfo_High & 0xF0) << 4;

  } else {

    PreferredResolution[0] = 720;
    PreferredResolution[1] = 480;

  }

  // (show actual VBE/EDID info)
  // TODO: Show more!!!

  if (VbeIsSupported == true) {
    Message(Info, "Preferred resolution is %d * %d.", PreferredResolution[0], PreferredResolution[1]);
  }

  // (get best mode)

  // This should definitely be a function - okay, it is one now, but *I still haven't tested
  // this super extensively, so do that first!!*

  uint16 BestVbeMode = 0xFFFF;
  volatile vbeModeInfoBlock BestVbeModeInfo;

  if (VbeIsSupported == true) {

    BestVbeMode = FindBestVbeMode((uint16*)(convertFarPtr(VbeInfo.VideoModeListPtr)), PreferredResolution[0], PreferredResolution[1]);
    GetVbeModeInfo(&BestVbeModeInfo, BestVbeMode);

    Message(Info, "Using mode %xh, with a %dx%d resolution and a %d-bit color depth.", BestVbeMode, BestVbeModeInfo.ModeInfo.X_Resolution, BestVbeModeInfo.ModeInfo.Y_Resolution, BestVbeModeInfo.ModeInfo.BitsPerPixel);

  } else {

    Message(Info, "Using VGA text mode (due to a lack of VBE support)");

  }




  // [SMBIOS]
  // F0000h to FFFFFh; this is pretty important, but it can be dealt with later on, for now
  // I mostly just want to check to see if SMBIOS even exists.

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get SMBIOS data");

  // (get the table itself)

  void* SmbiosEntryPoint = GetSmbiosEntryPointTable();
  bool SmbiosIsSupported = true;

  if (SmbiosEntryPoint == NULL) {
    SmbiosIsSupported = false;
  }

  // (show info)

  if (SmbiosIsSupported == true) {
    Message(Info, "SMBIOS appears to be supported (the entry point table is at %xh).", SmbiosEntryPoint);
  } else {
    Message(Warning, "SMBIOS appears to be unsupported.");
  }





  // [PCI]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get PCI-BIOS data");

  // -> int 1Ah, ax = B101h [PCI-related; *may* be important later on]
  // (get info)

  pciBiosInfoTable PciBiosTable;
  uint32 PciBiosReturnStatus = GetPciBiosInfoTable(&PciBiosTable);

  // (is it supported?)

  bool PciBiosIsSupported = true;

  if ((PciBiosReturnStatus & 0xFF00) != 0) {
    PciBiosIsSupported = false;
  }

  // (show info)

  if (PciBiosIsSupported == true) {

    Message(Info, "Successfully obtained information using the PCI BIOS interrupt call.");

  } else {

    Message(Warning, "Failed to obtain information using the PCI BIOS interrupt call.");
    Message(Info, "PCI BIOS may not be available on this system.");

  }





  // [Disk and filesystem drivers (this is the last thing to do)]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get EDD/FAT data");


  // (get a few initial variables)

  DriveNumber = InfoTable->DriveNumber;
  bool Edd_Valid = InfoTable->Edd_Valid;

  LogicalSectorSize = InfoTable->LogicalSectorSize;
  PhysicalSectorSize = InfoTable->PhysicalSectorSize;

  bool IsFat32 = InfoTable->Bpb_IsFat32;

  Message(Info, "Successfully obtained drive/EDD-related information.");

  // (get bpb)

  #define Bpb_Address (&InfoTable->Bpb[0])

  biosParameterBlock Bpb = *(biosParameterBlock*)(Bpb_Address);

  biosParameterBlock_Fat16 Extended_Bpb16 = *(biosParameterBlock_Fat16*)(Bpb_Address + 33);
  biosParameterBlock_Fat32 Extended_Bpb32 = *(biosParameterBlock_Fat32*)(Bpb_Address + 33);

  // (sanity-check the BPB)

  if (Bpb.BytesPerSector == 0) {
    Panic("Failed to obtain the BPB.", 0);
  } else {
    Message(Info, "Successfully obtained the BPB from the bootloader's info table.");
  }



  // (now, let's focus on getting fs info)
  // (now, let's focus on getting fs info)
  // (now, let's focus on getting fs info)

  // -> the total number of sectors within the partition, including reserved sectors

  uint32 TotalNumSectors = Bpb.NumSectors;

  if (TotalNumSectors == 0) {
    TotalNumSectors = Bpb.NumSectors_Large;
  }

  // -> the size of each FAT

  uint32 FatSize = Bpb.SectorsPerFat;

  if (FatSize == 0) {
    FatSize = Extended_Bpb32.SectorsPerFat;
  }

  // -> the number of root sectors

  uint32 NumRootSectors = ((Bpb.NumRootEntries * 32) + (LogicalSectorSize - 1)) / LogicalSectorSize;

  // -> the position of the first data sector, relative to the start of the partition

  uint32 DataSectorOffset = ((Bpb.NumFileAllocationTables * FatSize) + NumRootSectors) + Bpb.ReservedSectors;

  // -> the number of data (non-reserved + non-FAT) sectors in the partition

  uint32 NumDataSectors = (TotalNumSectors - DataSectorOffset);

  // -> the number of clusters in the partition

  uint32 NumClusters = (NumDataSectors / Bpb.SectorsPerCluster);

  // -> the cluster limit

  uint32 ClusterLimit = 0xFFF6;

  if (IsFat32 == true) {
    ClusterLimit = 0x0FFFFFF6;
  }




  // (now, let's actually load things)
  // (now, let's actually load things)
  // (now, let's actually load things)

  // -> first, we need to get the root cluster, along with the sector offset of that cluster

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

  // -> next, we need to search for the Boot/ directory, starting from the root directory,
  // like this:

  fatDirectory BootDirectory = FindDirectory(RootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, RootSectorOffset, "BOOT    ", "   ", true, IsFat32);
  uint32 BootCluster = GetDirectoryCluster(BootDirectory);

  if (ExceedsLimit(BootCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/.", 0);
  } else {
    Message(Kernel, "(debug) Found the Boot/ directory; yay");
  }



  // TODO:

  // -> Create a physical memory allocator. This doesn't have to be anything complicated, and
  // you don't need to implement anything to free memory either; just make sure that it can
  // actually allocate memory, and that it works on non-consecutive areas (above 1MiB).

  Putchar('\n', 0);

  uint64 AddressThing = 0x100000; // Needs to be >1MiB, since we wanna preserve everything below that
  uint32 SizeThing = 0x23456;

  uint64 Test = AllocFromUsableMmap(AddressThing, SizeThing, UsableMmap, NumUsableMmapEntries);
  Message(Info, "(debug) Tested out AllocFromUsableMmap()\n       (debug) (initial address: %xh, new address: [%xh, %xh])", (uint32)AddressThing, (uint32)(Test - SizeThing), (uint32)Test);

  uint64 Test2 = AllocFromUsableMmap(Test, 0x122A, UsableMmap, NumUsableMmapEntries);
  Message(Info, "(debug) Tested out AllocFromUsableMmap()\n       (debug) (initial address: %xh, new address: [%xh, %xh])", (uint32)Test, (uint32)(Test2 - 0x122A), (uint32)Test2);


  // -> Identity-map all of memory (or at least, all that actually matters). This generally
  // uses up to 0.2% of total memory, so for a 128MiB system, it'll occupy around 256KiB, give
  // or take.

  // -> Finally, actually load the rest of the bootloader and jump to it.
  // [TODO: ...well, that, lol.]



  // [For now, let's just leave things here]

  Debug = true;

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("January %i %x\n", 0x3F, 16, 0x2025);

  for(;;);

}
