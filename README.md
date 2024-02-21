# Serra
An x86 bootloader under construction.

&nbsp;

## Roadmap
- Bootsector **Done**
- Protected mode **Done**
- Set up a proper IDT **Done**

&nbsp;

- String/terminal functions **Done**
- Enabling the A20 line **Done**
- Interrupts and exceptions **Done (but a little bare-bones)**
- Some way of going to and from real mode **Done**
- E820 / memory map *Partly done - need to interpret the memory map still.*
- CPUID *Just starting out, still need to actually interpret the data.*
- ACPI
- VESA

&nbsp;

- Paging
- TSS, ring 0-3(?)
- Long mode

- FAT support? (need to work out BPB as well)
- EFI support?
- Probably some other filesystem
- Maybe some sort of standard protocol?
- Might want Multiboot support and stuff too

&nbsp;

## Disclaimer
This project has been released under the MIT license. For more information, please
refer to the accompanying license agreement. <3
