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

  // (This is also important - check for PAE, PSE and long mode support, so we know how to
  // implement paging; also, *64-bit support is mandatory*)

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
  } else {
    Panic("64-bit mode appears to be unsupported.", 0);
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


  // [Process the E820 memory map]

  // (TODO 1: add this to its own function, i guess..? in Mmap.c)
  // (TODO 2: also, add specific exceptions for areas under 1MiB, VESA video modes, etc.)

  Putchar('\n', 0);
  Message(Kernel, "Preparing to process the system's memory map.");

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




  // [Disk and filesystem drivers (this is the last thing to do)]

  Putchar('\n', 0);
  Message(Kernel, "Preparing to get EDD/FAT data");


  // (get a few initial variables)

  DriveNumber = InfoTable->DriveNumber;
  bool Edd_Enabled = InfoTable->Edd_Enabled;

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
  }

  // -> Then, Boot/Serra:

  fatDirectory SerraDirectory = FindDirectory(BootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "SERRA   ", "   ", true, IsFat32);
  uint32 SerraCluster = GetDirectoryCluster(SerraDirectory);

  if (ExceedsLimit(SerraCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/Serra/.", 0);
  }

  // -> Now, let's search for Boot/Serra/Kernel.elf.

  fatDirectory KernelDirectory = FindDirectory(SerraCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "KERNEL  ", "ELF", false, IsFat32);
  uint32 KernelCluster = GetDirectoryCluster(KernelDirectory);

  if (ExceedsLimit(KernelCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/Serra/Kernel.elf.", 0);
  } else {
    Message(Ok, "Successfully located Boot/Serra/Kernel.elf.");
  }
























  // [2 - Basically just paging(TM)]



  // [2.1] Allocate space for the 512 initial PML4 tables, and identity-map
  // the first 512GiB to 00h.low (the 1st PML4).

  Putchar('\n', 0);
  Message(Kernel, "Allocating space for the initial (identity-mapped) page tables.");

  // [2.1.1] Find a suitable starting point; although 1MiB is a common starting
  // point, it might not work everywhere, so we scan the usable mmap first.

  uint64 Offset = 0x100000;

  for (uint8 Entry = 0; Entry < NumUsableMmapEntries; Entry++) {

    if (UsableMmap[Entry].Base >= 0x100000) {

      Offset = UsableMmap[Entry].Base;
      break;

    }

  }

  // [2.1.k1] Allocate space for the kernel.
  // (Do not name this 'Kernel', GCC will override any Message(Kernel, ...) msgs lmao.)

  uintptr KernelPtr = (uintptr)(AllocateFromMmap(Offset, (uint32)(KernelDirectory.Size), false, UsableMmap, NumUsableMmapEntries));

  if (KernelPtr == 0) {

    Panic("Unable to allocate enough space for the kernel.", 0);

  } else {

    Offset = KernelPtr;
    KernelPtr -= KernelDirectory.Size;

    Message(Ok, "Allocated kernel between %xh and %xh (in pmem)", (uint32)KernelPtr, (uint32)Offset);

  }

  // [2.1.k2] Allocate space for the kernel *stack*.

  #define KernelStackSize 0x100000 // Must be a multiple of 4KiB
  uintptr KernelStack = (uintptr)(AllocateFromMmap(Offset, KernelStackSize, false, UsableMmap, NumUsableMmapEntries));

  if (KernelStack == 0) {

    Panic("Unable to allocate enough space for the kernel stack.", 0);

  } else {

    Offset = KernelStack;
    KernelStack -= KernelStackSize;

    Message(Ok, "Allocated kernel stack between %xh and %xh (in pmem)", (uint32)(KernelStack), (uint32)(Offset));

  }

  // [2.1.2] Allocate space for the 512 PML4 tables.

  uintptr Pml4 = (uintptr)(AllocateFromMmap(Offset, (512 * 8), true, UsableMmap, NumUsableMmapEntries));
  uint64* Pml4_Data;

  if (Pml4 == 0) {

    Panic("Unable to allocate enough space for the page tables.", 0);

  } else {

    Offset = Pml4;
    Pml4 -= (512 * 8); Pml4_Data = (uint64*)Pml4;

    Message(Ok, "Allocated PML4 space between %xh and %xh", (uint32)(Pml4), (uint32)(Offset));

  }






  #define UsableFlags (pagePresent | pageRw)
  #define IdmappedFlags (pagePresent | pageRw | pagePcd) // (for 2MiB pages, add pageSize)





  // Identity map woooo

  Putchar('\n', 0);
  Message(Kernel, "Preparing to map memory.");


  // (Identity-map *everything* up to IdentityMapThreshold in memory)

  #define IdentityMapThreshold 0x1000000 // (16 MiB)

  Offset = InitializePageEntries(0, 0, IdentityMapThreshold, Pml4_Data, IdmappedFlags, true, Offset, UsableMmap, NumUsableMmapEntries);
  Message(Ok, "Successfully identity mapped the first %d MiB of memory", (IdentityMapThreshold / 0x100000));


  // (Identity-map everything else in the *usable* memory map, above the threshold)
  // So, a reserved or missing area above it won't get mapped, but a free area will.

  for (uint16 Entry = 0; Entry < NumUsableMmapEntries; Entry++) {

    // Get variables

    uint64 Start = UsableMmap[Entry].Base;
    uint64 Size = UsableMmap[Entry].Limit;

    // Align to 2MiB (huge page) boundaries; more specifically, align start *down*,
    // but align size *up*

    Start -= (Start % 0x200000);

    if ((Size % 0x200000) != 0) {
      Size += (0x200000 - (Size % 0x200000));
    }

    // Initialize page entries

    Offset = InitializePageEntries(Start, Start, Size, Pml4_Data, IdmappedFlags, true, Offset, UsableMmap, NumUsableMmapEntries);
    Message(Ok, "Successfully identity mapped %d MiB area starting at %x:%xh", (uint32)(Size / 0x100000), (uint32)(Start >> 32), (uint32)(Start & 0xFFFFFFFF));

  }






  // [3.1] Okay; now, let's actually *load* the kernel into memory.

  Putchar('\n', 0);
  Message(Kernel, "Preparing to load and execute the kernel.");

  bool ReadFileSuccessful = ReadFile((void*)KernelPtr, KernelDirectory, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, IsFat32);

  if (ReadFileSuccessful == true) {
    Message(Ok, "Successfully loaded Boot/Serra/Kernel.elf to %xh.", (uint32)KernelPtr);
  } else {
    Panic("Failed to read Boot/Serra/Kernel.elf from disk.", 0);
  }


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
      Offset = InitializePageEntries(PhysAddress, Header->VirtAddress, Header->Size, Pml4_Data, UsableFlags, false, Offset, UsableMmap, NumUsableMmapEntries);

    }

  }

  // Todo: there seems to be some alignment issues? but let me see if it works okay

  Message(Kernel, "Mapping kernel stack; offset = %xh", (uint32)Offset);
  Offset = InitializePageEntries((uint64)KernelStack, (KernelHeader->Entrypoint - KernelStackSize), KernelStackSize, Pml4_Data, UsableFlags, false, Offset, UsableMmap, NumUsableMmapEntries);
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


  // (DEBUG)
  // Why doesn't this work!? (Or, better, it *does* work, but like.. why doesn't it
  // not work aaagh.)

  // (TODO: does this even work on real hardware? maybe, i hope)

  uint64 DebugPml4 = Pml4_Data[511];

  uint64* Pml3_Data = (uint64*)pageAddress(DebugPml4);
  uint64 DebugPml3 = Pml3_Data[0];

  uint64* Pml2_Data = (uint64*)pageAddress(DebugPml3);
  uint64 DebugPml2 = Pml2_Data[0];

  uint64* Pte_Data = (uint64*)pageAddress(DebugPml2);
  uint64 DebugPte = Pte_Data[0];

  Message(Kernel, "Debug pml4 = %x:%x, debug pml3 = %x:%x", (uint32)(DebugPml4 >> 32), (uint32)DebugPml4, (uint32)(DebugPml3 >> 32), (uint32)DebugPml3);
  Message(Kernel, "Debug pml2 = %x:%x, debug pte = %x:%x", (uint32)(DebugPml2 >> 32), (uint32)DebugPml2, (uint32)(DebugPte >> 32), (uint32)DebugPte);



  // [For now, let's just leave things here]

  Debug = true;


  char* Teststring = "This is also Serra! <3                                                          "
                     "(long mode kernel edition)                                                      "
                     "                                                                                ";


  Putchar('\n', 0);

  Printf("Hi, this is Serra! <3\n", 0x0F);
  Printf("April %i %x\n", 0x3F, 14, 0x2025);

  LongmodeStub((uintptr)Teststring, Pml4);


  for(;;);

}
