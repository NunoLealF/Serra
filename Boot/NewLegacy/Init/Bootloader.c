// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#include "../Shared/Stdint.h"
#include "Bootloader.h"

#ifndef __i686__
#error "This code is supposed to be compiled with an i686-elf cross-compiler."
#endif

/* void __attribute__((noreturn)) Init()

   Inputs:    (none)
   Outputs:   (none)

   Initializes the segment registers (in this case, DS, ES, FS, GS, and SS) with the data selector
   (10h) as set in our GDT, sets the stack location to 20000h (where it'll have 64KiB of space),
   and then jumps to our Bootloader() function.

   This function is defined by our linker to be at 7E00h in memory, which is where our first-stage
   bootloader (our MBR) jumps to after loading our second-stage bootloader into memory and enabling
   protected mode.

   As this is essentially the first code we execute after enabling protected mode, we need to set
   up a basic environment for our code - specifically, we need to set up the segment registers
   (except for CS), and set up a basic stack, which in our case is at 20000h.

*/

void __attribute__((noreturn)) Init(void) {

  // Set up registers (DS, ES, FS, GS, and SS)
  // We'll be setting these up with the data segment (10h in our GDT).

  __asm__("mov $0x10, %eax;"
          "mov %eax, %ds;"
          "mov %eax, %es;"
          "mov %eax, %fs;"
          "mov %eax, %gs;"
          "mov %eax, %ss");

  // Set up the stack at 20000h (by setting SP).

  __asm__("mov $0x20000, %esp");

  // Jump to our Bootloader() function.

  Bootloader();

}


/* void __attribute__((noreturn)) Bootloader()

   Inputs:    (none)
   Outputs:   (none)

   This is our second-stage bootloader's main function. We jump here after Init().

   Todo: Write a more concise description. (I feel like this is only really possible after actually
   finishing this part, lol)

*/

// 7C00h -> 7E00h: 1st stage bootloader
// 7E00h -> 9E00h: 2nd stage bootloader
// 9E00h -> AC00h: Shared real-mode code
// AC00h -> AE00h: Shared real-mode data
// AE00h -> 10000h: [This is basically just used to test things out; treat as empty, but volatile]
// 10000h -> 20000h: Stack
// 20000h -> ?????h: 3rd stage bootloader

// F000h (4 bytes): A20 test location, should probably change that.

void __attribute__((noreturn)) Bootloader(void) {

  // We've finally made it to our second-stage bootloader. We're in 32-bit x86 protected mode with
  // the stack at 20000h in memory, and our bootloader between 7E00h and CE00h in memory.

  // (Set up terminal)

  InitializeTerminal(80, 25, 0xB8000);
  ClearTerminal();

  Message(Kernel, "Successfully entered the second-stage bootloader.");

  Putchar('\n', 0);


  // (Set up A20)

  // At the moment, we can only reliably access up to the first MiB of data (from 00000h to FFFFFh).
  // This is because we haven't yet enabled the A20 line, which is a holdover from the 8086 days.

  // Before having loaded our second-stage bootloader, our bootsector actually tried to enable it
  // using the BIOS function int 15h, ax 2401h. However, this isn't guaranteed to work on all
  // systems.

  // Before we try to use any methods to enable the A20 line, we want to check if it's already been
  // enabled (by the firmware, or by the BIOS function we executed earlier). If so, we want to skip
  // to the next part of the bootloader.

  bool A20_EnabledByDefault = false;
  bool A20_EnabledByKbd = false;
  bool A20_EnabledByFast = false;

  Message(Kernel, "Preparing to enable the A20 line");

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
      Message(Kernel, "Attempting to enable the A20 line using the fast A20 method");

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

        Panic("Failed to enable the A20 line");

      }

    }

  }

  Putchar('\n', 0);




  // In order to be able to load things from disk, we need to know the current drive number,
  // which should (hopefully) be saved in the bootsector.

  // By default, the BIOS passes the current drive number as dl, but it doesn't actually store it
  // anywhere else; instead, our bootsector manually saves the value of dl right before the
  // bootsector signature (AA55h).

  // For now, we're just going to check if the bootsector is still in memory (by verifying the
  // signature), and check to see if the drive number is in a valid range - otherwise, we'll just
  // use the 'default' drive number (usually 80h).

  DriveNumber = *(uint8*)(0x7E00 - 3);
  bool DriveNumberIsValid = true;

  uint16 Signature = *(uint16*)(0x7E00 - 2);

  if (Signature != 0xAA55) {
    Panic("Bootsector/VBR hasn't been loaded into memory.");
  } else if (DriveNumber == 0x00) {
    DriveNumberIsValid = false;
  }

  // If the drive number saved in our bootsector is somehow invalid, then we're going to want
  // to just set it to the default 80h, and to also show a warning.

  if (DriveNumberIsValid == true) {

    Message(Ok, "Successfully got the current drive number.");
    Message(Info, "The current drive number is %xh.\n", DriveNumber);

  } else {

    Message(Warning, "Failed to get the current drive number; setting it to 80h.");
    DriveNumber = 0x80;

  }




  // Alright; now, we want to focus on getting information from our current drive - how large
  // is it, how many bytes are in a sector, how should we load our data?

  // We can get this information using the BIOS function (int 13h, ah 48h), which should
  // (theoretically) return a table in [ds:si] for the drive in the dl register:

  // [TODO: WRITE A BETTER COMMENT, C'MON C'MON]

  // (prepare)

  eddDriveParameters EDD_Parameters;

  Memset((void*)&EDD_Parameters, sizeof(eddDriveParameters), 0);
  EDD_Parameters.Size = sizeof(eddDriveParameters);

  // (actually do it!)

  realModeTable* Table = InitializeRealModeTable();

  Table->Eax = (0x48 << 8);
  Table->Edx = DriveNumber;

  Table->Ds = ((unsigned int)(&EDD_Parameters) >> 4);
  Table->Si = ((unsigned int)(&EDD_Parameters) & 0x0F);

  Table->Int = 0x13;

  Message(Kernel, "Preparing to get EDD/drive information.");
  RealMode();

  // (Any errors? If not, then carry on; otherwise, keep moving on with 'standard' values that
  // just hopelessly assume you have a big enough drive (and a valid BPB))

  bool EDD_Enabled = true;

  if (hasFlag(Table->Eflags, CarryFlag)) {
    EDD_Enabled = false;
  } else if (Table->Eax != (0x48 << 8)) {
    EDD_Enabled = false;
  }

  if (EDD_Enabled == true) {

    Message(Ok, "Successfully obtained EDD/drive data.");

  } else {

    Message(Warning, "Unable to get EDD data - relying on defaults from now on.");

    EDD_Parameters.Size = 0x1A;
    EDD_Parameters.Flags = 0xFFFF;
    EDD_Parameters.BytesPerSector = 512;

  }

  // (Finally, if EDD_Enabled is false, then let's just do a quick sanity check to see if the
  // disk read function is even working)

  if (EDD_Enabled == false) {

    uint16* SanityCheck = (uint16*)(0xAE00);
    *SanityCheck = 0xE621;

    Table = ReadSector(1, 0xAE00, 0);

    if (hasFlag(Table->Eflags, CarryFlag) == true) {
      Panic("Unable to successfully load data from the drive.");
    } else if (*SanityCheck == 0xE621) {
      Panic("Unable to successfully load data from the drive.");
    }

  }



  // Finally, let's load in the regular BPB, and see if it's even a proper FAT partition (if
  // not, then panic, since the next stage of the bootloader is a FAT file lol)

  // We don't need to load anything with the disk functions (yet!) since it's all in the
  // bootsector, which is *guaranteed* to be at 7C00h.

  biosParameterBlock Bpb = *(biosParameterBlock*)(0x7C00 + 3);

  biosParameterBlock_Fat16 Extended_Bpb16 = *(biosParameterBlock_Fat16*)(0x7C00 + 36);
  biosParameterBlock_Fat32 Extended_Bpb32 = *(biosParameterBlock_Fat32*)(0x7C00 + 36);

  Putchar('\n', 0);

  if (Bpb.BytesPerSector == 0) {
    Panic("Failed to detect a FAT partition.");
  } else {
    Message(Kernel, "Successfully detected a FAT partition.");
  }



  // Save the logical and physical sector size (the bytes per sector given by the BPB and by
  // the EDD/int13h functions respectively)

  LogicalSectorSize = Bpb.BytesPerSector;
  PhysicalSectorSize = EDD_Parameters.BytesPerSector;



  // (Get some variables from the BPB)

  // Now that we know that the BPB exists, and that it almost certainly represents a valid FAT
  // partition, we want to actually start reading the data off of it. Specifically, we're going to
  // focus on these specific variables:

  // -> The total number of sectors within the partition, including reserved sectors;

  uint32 TotalNumSectors = Bpb.NumSectors;

  if (TotalNumSectors == 0) {
    TotalNumSectors = Bpb.NumSectors_Large;
  }

  // -> The size of each FAT (file allocation table)

  uint32 FatSize = Bpb.SectorsPerFat;

  if (FatSize == 0) {
    FatSize = Extended_Bpb32.SectorsPerFat;
  }

  // -> The number of root sectors (if the filesystem is FAT32, then this is always zero);

  uint32 NumRootSectors = ((Bpb.NumRootEntries * 32) + (LogicalSectorSize - 1)) / LogicalSectorSize;

  // -> The position of the first data sector (relative to the start of the partition), along
  // with the number of data (non-reserved + non-FAT) sectors in the partition.

  uint32 DataSectorOffset = ((Bpb.NumFileAllocationTables * FatSize) + NumRootSectors) + Bpb.ReservedSectors;
  uint32 NumDataSectors = (TotalNumSectors - DataSectorOffset);

  // -> And, finally, the number of clusters within the partition.

  uint32 NumClusters = (NumDataSectors / Bpb.SectorsPerCluster);




  // Now that we've gone through this whole process, we can actually finally move on to
  // determining the filesystem type. It's actually relatively simple - we just need to look at
  // the number of clusters in the partition:

  // -> Any partition with less than 4085 clusters is guaranteed to be FAT12.
  // -> Any partition with between 4085 and 65524 clusters is guaranteed to be FAT16.
  // -> Any partition with more than 65524 clusters is guaranteed to be FAT32.

  bool PartitionIsFat32 = false;
  uint32 ClusterLimit = 0xFFF6;

  if (NumClusters < 4085) {

    Panic("FAT partitions with less than 4085 clusters are not supported.");

  } else if (NumClusters > 65524) {

    ClusterLimit = 0x0FFFFFF6; // (Caution, only the bottom 28 bits actually matter, 9FFFFFF5 should pass but 0FFFFFF6 shouldn't)
    PartitionIsFat32 = true;

  }

  Message(Info, "The current FAT partition has %d clusters (each %d bytes long).", NumClusters, (Bpb.SectorsPerCluster * LogicalSectorSize));




  // Now that we know the partition type, we can get the cluster (and the first sector of) the
  // root directory, like this:

  uint32 RootCluster;

  if (PartitionIsFat32 == false) {
    RootCluster = 2;
  } else {
    RootCluster = Extended_Bpb32.RootCluster;
  }


  // (test)



  Putchar('\n', 0);
  Message(Info, "Test: cluster number %d becomes %xh", 0, GetFatEntry(0, Bpb.HiddenSectors, Bpb.ReservedSectors, PartitionIsFat32));
  Message(Info, "Test: cluster number %d becomes %xh", 1, GetFatEntry(1, Bpb.HiddenSectors, Bpb.ReservedSectors, PartitionIsFat32));
  Message(Info, "Test: cluster number %d becomes %xh", RootCluster, GetFatEntry(RootCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, PartitionIsFat32));
  Message(Info, "Test: cluster number %d becomes %xh", 3, GetFatEntry(3, Bpb.HiddenSectors, Bpb.ReservedSectors, PartitionIsFat32));

  // Okay, we have RootSectorOffset, all that's really left to do now is to just follow the
  // cluster chain(s), and find Boot/Serra.bin.

  Putchar('\n', 0);
  Message(Kernel, "Preparing to locate the next stage of the bootloader.");

  // (Test: Try to use FindDirectory() to find Core.bin, which should be in the root folder,
  // hopefully)

  uint32 Core = FindDirectory(RootCluster, Bpb.SectorsPerCluster, Bpb.HiddenSectors, Bpb.ReservedSectors, DataSectorOffset, "CORE    ", "BIN", false, PartitionIsFat32);

  Printf("Core.bin (cluster number): %x\n", 0x03, Core);



  // Each entry is 32 bytes long: https://wiki.osdev.org/FAT
  // LFN support almost certainly isn't needed

  // It seems like each directory has its own cluster chain; in this case, for the root
  // directory, you start directly at the first (probably at some random) file or directory,
  // and then you just have to iterate through that until you find a folder named Boot, and
  // then, until you find a file named Serra.bin.


  // There's only like 1.5KiB of code space left, jesus, I better get this over with quickly
  // haha

  // [A few things to keep in mind]

  // -> The next stage will be a bin/elf file in a FAT16 or FAT32 filesystem, in the root
  // directory (this isn't EFI, no need to look through /BOOT/EFI..)

  // -> You don't really want a full-fledged FAT implementation, just enough to traverse the
  // root directory, find the next stage, load it, and then jump from there.




  // [For now, let's end things here]

  Putchar('\n', 0);

  Printf("Hiya, this is Serra! <3\n", 0x0F);
  Printf("July %i %x\n", 0x3F, 16, 0x2024);

  for(;;);

}
