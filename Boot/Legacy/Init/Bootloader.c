// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

/* void SaveState(), RestoreState()

   Inputs: (none)
   Outputs: (none)

   These two functions are called by Shared/Rm/RmWrapper.c, and they basically just save and
   restore the state of the system (for example, they might restore the IDT, paging, etc.)

   In this stage of the bootloader however, we don't need to do anything, so for now, these
   functions just remain empty.

*/

void SaveState(void) {
  return;
}

void RestoreState(void) {
  return;
}


/* [[noreturn]] void S2Init()

   Inputs:    (none)
   Outputs:   (none)

   Initializes the segment registers (in this case, DS, ES, FS, GS, and SS) with the data selector
   (10h) as set in our GDT, sets the stack location to 20000h (where it'll have 64KiB of space),
   and then jumps to our S2Bootloader() function.

   This function is defined by our linker to be at 7E00h in memory, which is where our first-stage
   bootloader (our MBR) jumps to after loading our second-stage bootloader into memory and enabling
   protected mode.

   As this is essentially the first code we execute after enabling protected mode, we need to set
   up a basic environment for our code - specifically, we need to set up the segment registers
   (except for CS), and set up a basic stack, which in our case is at 20000h.

*/

[[noreturn]] void S2Init(void) {

  // Set up registers (DS, ES, FS, GS, and SS)
  // We'll be setting these up with the data segment (10h in our GDT).

  __asm__ __volatile__ ("mov $0x10, %eax;"
          "mov %eax, %ds;"
          "mov %eax, %es;"
          "mov %eax, %fs;"
          "mov %eax, %gs;"
          "mov %eax, %ss");

  // Set up the stack at 20000h (by setting SP).

  __asm__ __volatile__ ("mov $0x20000, %esp");

  // Jump to our second-stage bootloader's main function.

  S2Bootloader();

}


/* [[noreturn]] void S2Bootloader()

   Inputs:    (none)
   Outputs:   (none)

   This is our *second-stage* bootloader's main function. We jump here after S2Init(),
   which initializes the segment registers.

   This stage of the bootloader occupies the space between 7E00h and 9E00h in memory, and
   its only role is to collect information about the system, and to jump to the next stage
   of the bootloader (a file named 'Boot/Bootx32.bin' in a FAT filesystem).

*/

[[noreturn]] void S2Bootloader(void) {

  // We've finally made it to our second-stage bootloader; we're in 32-bit x86 protected
  // mode with the stack at 20000h in memory, and our bootloader between 7E00h and 9E00h in
  // memory (leaving us with a total of 8KiB of space).


  // First, we'll want to set up the Debug flag/variable, which dictates whether
  // non-important messages should be shown or not. It's hardcoded at a specific offset
  // in our bootsector, so we only need to do this:

  #define DebugOffset 4
  #define PartitionEntryOffset 20

  // (Find partition table entry in bootsector, and set up PartitionLba)

  mbrPartitionEntry PartitionEntry = *(mbrPartitionEntry*)(0x7E00 - PartitionEntryOffset);
  bool PartitionEntryIsValid;

  if ((PartitionEntry.Attributes | (1 << 7)) != 0) {

    PartitionEntryIsValid = true;
    PartitionLba = PartitionEntry.Lba;

  } else {

    PartitionEntryIsValid = false;
    PartitionLba = 0;

  }

  // (Find Debug flag in bootsector)

  uint8 UseDebugFlag = *(uint8*)(0x7E00 - DebugOffset);

  if (UseDebugFlag == 0x00) {
    Debug = false;
  } else {
    Debug = true;
  }


  // Next, we can go ahead and set up the terminal, and then display a message showing
  // that we've successfully entered the second-stage bootloader.

  InitializeTerminal(80, 25, 0xB8000);
  ClearTerminal();

  Message(Boot, "Successfully entered the second-stage bootloader.");

  Putchar('\n', 0);


  // Now, we can get started on the next step - loading the next stage of the bootloader and
  // jumping to it. In order to be able to do that, we need to be able to read bytes from
  // disk, and later on, also interpret a FAT filesystem.

  // For now, we just want to collect some important data - specifically, we want to figure out
  // the current drive number (which is also saved in the bootsector, at a given offset):

  // (Update: This actually used to check for an invalid drive number, but not
  // only did it not work for floppy emulation (which some PCs use), it also
  // doesn't make sense because *if we're here, then the bootsector used that
  // drive number already* lol)

  DriveNumber = *(uint8*)(0x7E00 - 3);
  uint16 Signature = *(uint16*)(0x7E00 - 2);

  if (Signature != 0xAA55) {
    Panic("Bootsector/VBR hasn't been loaded into memory.", 0);
  } else {
    Message(Ok, "Successfully got the current drive number (%xh).", DriveNumber);
  }

  // Alright; now, we want to focus on getting information from our current drive - how large
  // is it, how many bytes are in a sector, how should we load our data?

  // We can get this information using the BIOS function (int 13h, ah 48h), which should
  // return a table (as in eddDriveParameters) in the address at [ds:si], for the drive in
  // the dl register.

  eddDriveParameters EddParameters;

  Memset((void*)&EddParameters, sizeof(eddDriveParameters), 0);
  EddParameters.Size = sizeof(eddDriveParameters);

  // (In order to actually call a BIOS function from protected mode, we'll use the real-mode
  // functions at Shared/Rm/ to do our work for us)

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = (0x48 << 8);
  Table->Edx = DriveNumber;

  Table->Ds = (uint16)((int)(&EddParameters) >> 4);
  Table->Si = (uint16)((int)(&EddParameters) & 0x0F);

  Table->Int = 0x13;

  Message(Boot, "Preparing to get EDD/drive information.");
  RealMode();

  // (Any errors? If not, then carry on; otherwise, keep moving on with 'standard' values that
  // just hopelessly assume you have a big enough drive (and a valid BPB))

  bool EddEnabled = true;

  if (hasFlag(Table->Eflags, CarryFlag)) {
    EddEnabled = false;
  } else if (Table->Eax != (0x48 << 8)) {
    EddEnabled = false;
  }

  if (EddEnabled == true) {

    Message(Ok, "Successfully obtained EDD/drive data.");

  } else {

    Message(Warning, "Unable to get EDD data - relying on defaults from now on.");

    EddParameters.Size = 0x1A;
    EddParameters.Flags = 0xFFFF;
    EddParameters.BytesPerSector = 512;

  }

  // (Finally, if EddEnabled is false, then let's just do a quick sanity check to see if the
  // disk read function is even working)

  if (EddEnabled == false) {

    #define SanityCheckValue 0xE621

    uint16* SanityCheck = (uint16*)(0xAE00);
    *SanityCheck = SanityCheckValue;

    Table = ReadSector(1, 0xAE00, 0);

    if (hasFlag(Table->Eflags, CarryFlag) == true) {
      Panic("Unable to successfully load data from the drive.", 0);
    } else if (*SanityCheck == SanityCheckValue) {
      Panic("Unable to successfully load data from the drive.", 0);
    }

  }


  // Now that we've obtained the information we needed from our drive, we can start working
  // on the next part - reading from the filesystem.

  // In order to do that, we first need to read data from the BPB (the BIOS Parameter Block),
  // which is a table in our bootsector (which is already in memory, from 7C00h to 7E00h),
  // like this:

  #define BpbAddress 0x7C00

  biosParameterBlock Bpb = *(biosParameterBlock*)(BpbAddress + 3);

  [[maybe_unused]] biosParameterBlock_Fat16 ExtendedBpb_16 = *(biosParameterBlock_Fat16*)(BpbAddress + 36);
  [[maybe_unused]] biosParameterBlock_Fat32 ExtendedBpb_32 = *(biosParameterBlock_Fat32*)(BpbAddress + 36);

  Putchar('\n', 0);

  if (Bpb.BytesPerSector < 512) {
    Panic("Failed to detect a FAT partition.", 0);
  } else {
    Message(Boot, "Successfully detected a FAT partition.");
  }


  // Next, we'll want to keep track of the physical sector size (specified by the system, and
  // as used by BIOS functions), and the logical sector size (as specified in the BPB).

  LogicalSectorSize = Bpb.BytesPerSector;
  PhysicalSectorSize = EddParameters.BytesPerSector;


  // After that, we'll want to actually start *reading* from the BPB.

  // At this point, we know that the BPB exists, and that it almost certainly represents a valid
  // FAT filesystem of some sort, but we don't know anything else, so we'll be collecting the
  // following variables:

  // (The total number of sectors within the partition, including reserved sectors)

  uint32 TotalNumSectors = Bpb.NumSectors;

  if (TotalNumSectors == 0) {
    TotalNumSectors = Bpb.NumSectors_Large;
  }

  // (The size of each FAT (file allocation table))

  uint32 FatSize = Bpb.SectorsPerFat;

  if (FatSize == 0) {
    FatSize = ExtendedBpb_32.SectorsPerFat;
  }

  // (The number of root sectors (if the filesystem is FAT32, then this is always zero))

  uint32 NumRootSectors = ((Bpb.NumRootEntries * 32) + (LogicalSectorSize - 1)) / LogicalSectorSize;

  // (The position of the first data sector (relative to the start of the partition), along
  // with the number of data (non-reserved + non-FAT) sectors in the partition)

  uint32 DataSectorOffset = ((Bpb.NumFileAllocationTables * FatSize) + NumRootSectors) + Bpb.ReservedSectors;
  uint32 NumDataSectors = (TotalNumSectors - DataSectorOffset);

  // (And finally, the number of clusters within the partition)

  uint32 NumClusters = (NumDataSectors / Bpb.SectorsPerCluster);


  // Now that we've collected that data, we can start determining the filesystem type. This
  // is actually *relatively* simple, as we just need to look at the number of clusters in
  // the partition:

  // -> Any partition with less than 4085 clusters is guaranteed to be FAT12.
  // -> Any partition with between 4085 and 65524 clusters is guaranteed to be FAT16.
  // -> Any partition with more than 65524 clusters is guaranteed to be FAT32.

  bool PartitionIsFat32 = false;
  uint32 ClusterLimit = 0xFFF6;

  if (NumClusters < 4085) {

    Panic("FAT partitions with less than 4085 clusters are not supported.", 0); // FAT12 isn't supported

  } else if (NumClusters > 65524) {

    ClusterLimit = 0x0FFFFFF6; // (Caution, only the bottom 28 bits actually matter, 9FFFFFF5 should pass but 0FFFFFF6 shouldn't)
    PartitionIsFat32 = true;

  }

  Message(Info, "The current FAT partition has %d clusters (each %d bytes long).", NumClusters, (Bpb.SectorsPerCluster * LogicalSectorSize));


  // Finally, now that we know all of these variables, we can actually start reading from the
  // partition!

  Putchar('\n', 0);
  Message(Boot, "Preparing to jump to the next stage of the bootloader.");

  // (First, we need to get the root cluster, along with the sector offset of that cluster)

  uint32 RootCluster;

  if (PartitionIsFat32 == false) {
    RootCluster = 2;
  } else {
    RootCluster = ExtendedBpb_32.RootCluster;
  }

  uint32 RootSectorOffset = DataSectorOffset;

  if (PartitionIsFat32 == false) {
    RootSectorOffset -= NumRootSectors;
  }

  // (Next, we need to search for the Boot/ directory (starting from the root directory), like
  // this (we use the functions in Shared/Disk/Disk.h))

  fatDirectory BootDirectory = FindDirectory(RootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, RootSectorOffset, "BOOT    ", "   ", true, PartitionIsFat32);
  uint32 BootCluster = GetDirectoryCluster(BootDirectory);

  if (ExceedsLimit(BootCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/.", 0);
  }

  // (After that, we need to search for a file called 'Bootx32.bin' from within the Boot/ folder,
  // this time using the DataSectorOffset instead of the RootSectorOffset)

  fatDirectory BootloaderDirectory = FindDirectory(BootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "BOOTX32 ", "BIN", false, PartitionIsFat32);
  uint32 BootloaderCluster = GetDirectoryCluster(BootloaderDirectory);

  if (ExceedsLimit(BootloaderCluster, ClusterLimit)) {
    Panic("Failed to locate Boot/Bootx32.bin.", 0);
  } else {
    Message(Ok, "Located Boot/Bootx32.bin.");
  }


  // Now, we can actually go ahead and load the file from disk. Since we know the cluster
  // number of the "Boot/Bootx32.bin" file, we can just put it into ReadFile(), and it'll read
  // everything for us, like this:

  // (Our third-stage bootloader will occupy the space from 20000h onwards)

  #define BootloaderAddress 0x20000

  bool ReadFileSuccessful = ReadFile((void*)BootloaderAddress, BootloaderDirectory, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, PartitionIsFat32);

  if (ReadFileSuccessful == true) {
    Message(Ok, "Successfully read Boot/Bootx32.bin to %xh.", BootloaderAddress);
  } else {
    Panic("Failed to read Boot/Bootx32.bin from disk.", 0);
  }



  // Finally, now that we've loaded the third-stage bootloader, all that's left to do is to
  // fill out the bootloader info table (which passes on some of the information we've
  // gathered off to the next stage), and jump to the next stage.

  // First, we need to create and fill out the table, like this (see Shared/InfoTables.h
  // for more details):

  bootloaderInfoTable* InfoTable = (bootloaderInfoTable*)(InfoTableLocation);

  // (Fill out table info)

  InfoTable->Signature = 0x65363231;
  InfoTable->Version = 1;

  InfoTable->Size = sizeof(bootloaderInfoTable);

  // (Fill out system info)

  InfoTable->SystemInfo.Debug = Debug;

  // (Fill out disk/EDD info)

  InfoTable->DriveNumber = DriveNumber;
  InfoTable->EddEnabled = EddEnabled; // This should be whether EDD is enabled

  InfoTable->LogicalSectorSize = LogicalSectorSize;
  InfoTable->PhysicalSectorSize = PhysicalSectorSize;

  Memcpy(&InfoTable->EddInfo, &EddParameters, sizeof(InfoTable->EddInfo));

  // (Fill out filesystem/BPB info)

  InfoTable->BpbIsFat32 = PartitionIsFat32;
  Memcpy(&InfoTable->Bpb, (void*)(BpbAddress + 3), sizeof(InfoTable->Bpb));

  InfoTable->MbrEntryIsValid = PartitionEntryIsValid;
  InfoTable->PartitionEntry = PartitionEntry;
  InfoTable->PartitionLba = PartitionLba;

  // (Fill out terminal info)

  Memcpy(&InfoTable->TerminalInfo, &TerminalTable, sizeof(InfoTable->TerminalInfo));


  // And now, all that's left to do is to jump to the next stage (using inline assembly).

  // (In the event that the function returns for some reason, we'll just print out an error
  // and halt)

  __asm__ __volatile__ ("call *%0" : : "r"(BootloaderAddress));

  Panic("Failed to load the third-stage bootloader.", 0);
  for(;;);

}
