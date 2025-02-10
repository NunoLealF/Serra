## Boot/ roadmap <sup>(Legacy/BIOS-only for now)</sup>

(Disclaimer: Boot/Legacy/ is for the BIOS bootloader, Boot/Efi is for the
future UEFI bootloader, and Common/ is for the common kernel/manager)

### 1st stage (7C00h -> 7E00h)

- Bootsector **Done**
- Protected mode **Done**
- Set up a proper GDT **Done**

### 2nd stage (7E00h -> 9E00h)

- String/terminal functions **Done, and now with printf!**
- Some way of going to and from real mode **Done**
- Basic disk read support (using EDD / int 13h) **Done**
- Basic FAT filesystem support (FAT16 and FAT32) **Done, read-only**
- Set up a proper IDT **Done**
- Transferring all the info from the above to the next stage **Done, a little barebones though**
- Loading the next stage **Done**

### 3rd stage (20000h -> ?????h)

- String/terminal functions **Done, and now with printf!**
- Some way of going to and from real mode **Done**
- Interrupts and exceptions **Done (but a little bare-bones)**
- E820 / memory map **Done**
- CPUID ***Done (might be incomplete)***
- ACPI **Done, if a little bare-bones**
- VESA/EDID **Done**
- SMBIOS **Done (but a little bare-bones)**
- Disk drivers (int13h) ***Close to done** (read-only, might need to fix bugs)*
- Filesystem drivers ***Close to done** (read-only, might need to fix bugs)*
- Writing a (simple) physical memory manager *Initial implementation*
- Paging, 32-bit and PAE/long-mode *Planning stages, long-mode support is needed*
- Other miscellaneous things *In progress*
- Loading the next stage (with long mode!)

### Common stage (?????h)

- Basically all of the above, but with long-mode support, and also with an EFI stub that
does the same job as the BIOS/Legacy bootloader stub.

- Should probably work on some sort of standard protocol (maybe reuse Multiboot/Limine?)

- **Disclaimer: pretty sure this is in Common/, not in the Boot/ folder**

&nbsp;

## Disclaimer

This project has been released under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the accompanying license agreement. <3

<sub>(Disclaimer: this project is not affiliated with MIT)</sub>

&nbsp;

*(last updated on February 10th 2025)*
