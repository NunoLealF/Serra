# Serra
An x86 bootloader under construction <3

&nbsp;

## Roadmap

### 1st stage (7C00h -> 7E00h)

- Bootsector **Done**
- Protected mode **Done**
- Set up a proper IDT **Done**

### 2nd stage (7E00h -> 10000h)

- String/terminal functions **Done, and now with printf!**
- Enabling the A20 line **Done**
- Interrupts and exceptions **Done (but a little bare-bones)**
- Some way of going to and from real mode **Done**
- E820 / memory map *Partly done - need to interpret the memory map still.*
- CPUID *Also partly done, still need to interpret the data.*
- VESA/VBE
- Transferring all the info from the above to the next stage

### 2.5th stage (20000h -> 80000h)

- ACPI
- Paging
- PCI
- Storage (ATAPI, AHCI, NVMe, Floppy, etc.)
- An actual memory manager
- Filesystem (FAT) support? (Need to work out BPB as well)

### 3rd+ stages (???)

- Long mode
- TSS, ring 0-3(?)
- EFI support?
- Probably some other filesystem
- Maybe some sort of standard protocol?
- Might want Multiboot support and stuff too

&nbsp;

## Disclaimer
This project has been released under the MIT license. For more information, please
refer to the accompanying license agreement. <3
